// =====================================================================
// DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS.cpp
// =====================================================================
// LUNA project, K(13) >= 1155 record attempt.
// Sobol-F55 inspired cold-start SA from non-ZE99 seed (or ZE99+1 warm)
// with virgin vocabulary primitives Move P / Move Q adapted to sphere
// packing geometry. Heavy run, designed for 4-8 hours Mac M2 25% CPU.
//
// Architecture summary:
//   - Pool: ZE99 (1154 vectors, Z[sqrt(3)]) + cross-radical extensions
//   - State: subset S of |S| = N_target = 1155 vectors from pool
//   - Cost: number of pairs (i,j) in S with ip(v_i, v_j) > 8 + EPS
//   - Goal: cost = 0 with |S| = 1155 -> world record K(13) >= 1155
//   - Vocabulary: Move S (single-replace), Move P1/P2 (atomic dual), 
//                 Move Q (rotational), Move K (kick), Move R (soft restart)
//   - 5-chain Parallel Tempering, cold-shifted ladder
//   - Triple-rescue + adaptive promote
//   - MIX_RECAL every 30000 iters
//   - Auto-extend up to 6 times
//   - D190 verify scaffolding (writes pool dumps + final_audit at cierre)
//
// Author: Fresh-eyes Claude (Constructor candidate, post-Trincanero
//         Jefe self-jubilation V6). Architect: Rafa, Madrid.
// Date: 06 May 2026 deep night, Madrid.
// Build: g++ -O3 -march=native -std=c++17 -funroll-loops -Wall -Wextra
//        -o DOSPIRATAS DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS.cpp
// Run:  cd ~/Downloads && caffeinate -dims taskpolicy -c utility ./DOSPIRATAS
//       cojones_puros_final_1154.txt 2>&1 | tee DOSPIRATAS_run.log
// =====================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <ctime>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <climits>

// =====================================================================
// Constants and configuration
// =====================================================================
static constexpr int DIM            = 13;
static constexpr int N_TARGET       = 1155;     // record threshold (1154 = ZE99)
static constexpr double NORM2       = 16.0;     // canonical kissing-dim-13 normalization
static constexpr double IP_THRESH   = 8.0;      // kissing condition: ip <= 8
static constexpr double EPS         = 1e-6;     // float tolerance for Z[sqrt(3)]

static constexpr int N_CHAINS       = 5;
// V3 honest record-attack ladder: chain 4 at T=1.40 crosses Δ=+5 barriers
// with prob 2.8%, Δ=+8 with 0.3%. Without that, engine cannot escape the
// plateau=4 basin that v2 saturates in <350k iters.
// Cold-shifted preserved: chain 0 at T=0.05 retains discoveries via PT swap.
static constexpr double T_LADDER[N_CHAINS]   = {0.05, 0.15, 0.40, 0.80, 1.40};
static constexpr int    TABU_CAPS[N_CHAINS]  = {65, 60, 55, 45, 30};

// V3 honest record-attack budget: Sobol-grade investment.
// 16M iters/chain × 5 chains = 80M base + up to 8 × 2M = 96M total upper.
// Mac M2 25% CPU at ~25-30k iters/s → 6-10h wall-clock for cierre natural.
static constexpr long long ITERS_PER_CHAIN   = 16'000'000LL;
static constexpr long long EXTEND_BY         = 2'000'000LL;
static constexpr int       MAX_EXTENSIONS    = 8;
static constexpr int       MAX_REHEATS       = 200;

static constexpr int       MIX_RECAL_ITER    = 30'000;
static constexpr int       MIX_RECAL_WINDOW  = 30'000;
static constexpr double    MIX_RECAL_BUMP_UP = 1.5;
static constexpr double    MIX_RECAL_BUMP_DN = 0.5;

static constexpr int       RESCUE_PLATEAU_1  = 5'000;
static constexpr int       RESCUE_PLATEAU_2  = 2'000;
static constexpr int       RESCUE_PLATEAU_3  = 4'000;
static constexpr int       RESCUE_PROMOTE    = 7'500;

static constexpr int       PT_SWAP_INTERVAL  = 2'000;

static constexpr int       POLISH_PASSES_MAX = 15;

// Cost-probe filter thresholds (Sobol decision matrix transferred)
static constexpr double F1_KILL       = 0.03;   // <3% acceptance -> kill
static constexpr double F1_ORNAMENT   = 0.06;   // 3-6% -> ornamental
static constexpr double F2_ORNAMENT   = 0.001;  // <0.1% productive -> ornamental
static constexpr double F2_PROBE      = 0.005;  // 0.1-0.5% -> probe-mass
                                                //   >=0.5% -> full hammer

static constexpr int    F1_TRIALS     = 2000;
static constexpr int    F2_TRIALS     = 2000;
static constexpr double T_PROBE_F1    = 0.30;
static constexpr double T_PROBE_F2    = 0.08;

// RNG seed: hex-only encoding of "DOS PIRATAS COJONES PUROS"
// (only chars 0-9, A-F valid in hex literals)
static constexpr uint64_t RNG_SEED    = 0xD05B17A5C0F0E175ULL;

// =====================================================================
// Vector type and helpers
// =====================================================================
struct Vec {
    std::array<double, DIM> c;
    Vec() { c.fill(0.0); }
    double norm2() const {
        double s = 0.0;
        for (int i = 0; i < DIM; i++) s += c[i]*c[i];
        return s;
    }
    bool operator==(const Vec& o) const {
        for (int i = 0; i < DIM; i++) {
            if (std::abs(c[i] - o.c[i]) > EPS) return false;
        }
        return true;
    }
};

inline double ip(const Vec& a, const Vec& b) {
    double s = 0.0;
    for (int i = 0; i < DIM; i++) s += a.c[i]*b.c[i];
    return s;
}

inline bool kissing_pair(const Vec& a, const Vec& b) {
    return ip(a, b) <= IP_THRESH + EPS;
}

// =====================================================================
// Vec hash for O(1) lookup (V2 — post-audit fix O1)
//
// AUDIT FINDING: Move Q exact-match search was O(P) per try, ~10% CPU waste.
// FIX: bit_cast each double to uint64_t and combine. Since vectors live in
// Z[√3] (rational + sqrt(3) components), and ZE99 + Pattern A/B/E generates
// repeatable bit patterns, exact bit equality of double IS reliable for
// dedup in this pool.
// =====================================================================
struct VecHash {
    size_t operator()(const Vec& v) const noexcept {
        size_t h = 0xCBF29CE484222325ULL;  // FNV offset basis
        for (int i = 0; i < DIM; i++) {
            uint64_t bits;
            std::memcpy(&bits, &v.c[i], sizeof(double));
            h ^= bits + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
        }
        return (size_t)h;
    }
};
struct VecEqExact {
    bool operator()(const Vec& a, const Vec& b) const noexcept {
        for (int i = 0; i < DIM; i++) {
            if (std::memcmp(&a.c[i], &b.c[i], sizeof(double)) != 0) return false;
        }
        return true;
    }
};
using VecIndexMap = std::unordered_map<Vec, int, VecHash, VecEqExact>;

// =====================================================================
// Pool loader (reads CSV-style file from ZE99 pool path)
// =====================================================================
static std::vector<Vec> load_pool(const char* path) {
    std::vector<Vec> pool;
    std::ifstream fin(path);
    if (!fin) {
        fprintf(stderr, "[FATAL] cannot open pool file: %s\n", path);
        exit(1);
    }
    std::string line;
    int linenum = 0;
    while (std::getline(fin, line)) {
        linenum++;
        if (line.empty()) continue;
        // skip comment lines
        size_t firstnonspace = line.find_first_not_of(" \t");
        if (firstnonspace == std::string::npos) continue;
        if (line[firstnonspace] == '#') continue;

        Vec v;
        std::stringstream ss(line);
        std::string token;
        int k = 0;
        while (std::getline(ss, token, ',')) {
            if (k >= DIM) break;
            try {
                v.c[k] = std::stod(token);
            } catch (...) {
                fprintf(stderr, "[FATAL] parse error line %d token '%s'\n",
                        linenum, token.c_str());
                exit(1);
            }
            k++;
        }
        if (k != DIM) {
            fprintf(stderr, "[FATAL] line %d has %d coords, expected %d\n",
                    linenum, k, DIM);
            exit(1);
        }
        // sanity: norm² == 16
        double n2 = v.norm2();
        if (std::abs(n2 - NORM2) > EPS) {
            fprintf(stderr, "[FATAL] line %d norm²=%.6f != %.1f\n",
                    linenum, n2, NORM2);
            exit(1);
        }
        pool.push_back(v);
    }
    return pool;
}

// =====================================================================
// Cross-radical pool extension (V2 — post-audit fix Aguero #1)
//
// AUDIT FINDING (Rompehuevos Claude): original Pattern A (4 coords ±2,
// rest 0) generates vectors entirely INSIDE F-MASTER territory (Wall #1).
// Empirical check: min violations vs ZE99 = 4, mean 12.49, 0 candidates
// with 0 violations. Re-testing F-MASTER under another name.
//
// V2 fix: split N_EXTRA budget across THREE patterns:
//   Pattern A (25%): 4 coords ±2 — F-MASTER subset, kept for completeness.
//   Pattern B (25%): 1 coord ±2√3, 1 coord ±2 — sqrt3 mixed, novel scope.
//   Pattern E (50%): rotational images R·v for v ∈ ZE99, R structured
//                    coordinate-axis 90° rotations producing v' ∉ ZE99.
//                    These have provenance from ZE99-structure rather
//                    than disjoint algebraic land. F-cinco closure
//                    showed Q+Q√3-rotated copies don't help; but here
//                    we use IMAGES of individual vectors not full copies,
//                    and we mix them with Pattern A/B in the pool — the
//                    runtime SA dynamics decide which combinations matter.
// =====================================================================
static std::vector<Vec> extend_pool_cross_radical(
    const std::vector<Vec>& base,
    std::mt19937_64& rng,
    int n_extra)
{
    std::vector<Vec> out = base;
    out.reserve(base.size() + n_extra);

    const double S3 = std::sqrt(3.0);
    std::uniform_int_distribution<int> coord_dist(0, DIM-1);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    std::uniform_int_distribution<int> base_dist(0, (int)base.size() - 1);

    auto try_insert = [&](const Vec& v) -> bool {
        for (const auto& w : out) {
            if (v == w) return false;
        }
        out.push_back(v);
        return true;
    };

    // Budget split
    // V3 split: A=15%, B=15%, E=30%, F=40%. Pattern F earns largest share
    // because its empirical violation profile vs ZE99 is best (mean 6.31 vs
    // A=12.49, E=18.13). target_F is computed inside the F branch.
    int target_A = (n_extra * 15 + 99) / 100;
    int target_B = (n_extra * 15 + 99) / 100;
    int target_E = (n_extra * 30 + 99) / 100;
    int gen_A = 0, gen_B = 0, gen_E = 0;

    // ---- Pattern A: 4 coords ±2, rest 0 ----
    {
        int attempts = 0;
        const int MAX_ATTEMPTS = target_A * 50 + 100;
        while (gen_A < target_A && attempts < MAX_ATTEMPTS) {
            attempts++;
            Vec v;
            std::array<int, DIM> mask{};
            int picked = 0;
            while (picked < 4) {
                int idx = coord_dist(rng);
                if (mask[idx] == 0) {
                    mask[idx] = (sign_dist(rng) ? 2 : -2);
                    picked++;
                }
            }
            for (int i = 0; i < DIM; i++) v.c[i] = (double)mask[i];
            if (try_insert(v)) gen_A++;
        }
    }

    // ---- Pattern B: 1 coord ±2√3, 1 coord ±2 (norm² = 12 + 4 = 16) ----
    {
        int attempts = 0;
        const int MAX_ATTEMPTS = target_B * 20 + 100;
        while (gen_B < target_B && attempts < MAX_ATTEMPTS) {
            attempts++;
            Vec v;
            int i1 = coord_dist(rng);
            int i2 = coord_dist(rng);
            if (i1 == i2) continue;
            v.c[i1] = (sign_dist(rng) ? 2.0*S3 : -2.0*S3);
            v.c[i2] = (sign_dist(rng) ? 2.0    : -2.0);
            if (try_insert(v)) gen_B++;
        }
    }

    // ---- Pattern E: rotational image of ZE99 vector ----
    // Apply 90° rotation in coord pair (i,j): (a,b) -> (b,-a) or (-b,a)
    // to v ∈ base. Insert R·v if not in pool already.
    {
        int attempts = 0;
        const int MAX_ATTEMPTS = target_E * 20 + 100;
        while (gen_E < target_E && attempts < MAX_ATTEMPTS) {
            attempts++;
            int b_idx = base_dist(rng);
            int i = coord_dist(rng);
            int j = coord_dist(rng);
            if (i == j) continue;
            int sign_choice = sign_dist(rng);
            Vec v = base[b_idx];
            double a = v.c[i], b = v.c[j];
            if (sign_choice == 0) {
                v.c[i] = b;
                v.c[j] = -a;
            } else {
                v.c[i] = -b;
                v.c[j] = a;
            }
            // sanity: norm² preserved by rotation
            // (mathematically guaranteed; we trust it)
            if (try_insert(v)) gen_E++;
        }
    }

    // ---- Pattern F V3: structural difference of ZE99 pairs at IP=8 ----
    //
    // CONSTRUCTOR INSIGHT: u = v - w where v,w ∈ ZE99 with ⟨v,w⟩ = 8.
    // norm²(u) = 16 + 16 - 16 = 16 ✓ (kissing-norm preserved).
    // Empirical study (60k+ such pairs in ZE99) yields 9540 unique new
    // candidates. Profile vs ZE99: min violations = 1, mean = 6.31, with
    // 1341 candidates at ≤5 violations.
    //
    // This is FAR superior to Pattern A (mean 12.49) or Pattern E (mean
    // 18.13). Pattern F lives in the structural "halfway" geometry between
    // ZE99 vectors that touch at IP=8 — exactly the regime where 1155-th
    // vectors might exist if they exist anywhere.
    //
    // This is the territory NO LUNA engine has touched. F-MASTER tested
    // natural alphabets; F-cinco tested rotational copies; F-cross tested
    // cross-family low-cset; F-crosscoupled tested block-pair coupling.
    // None tested structural pair-differences. Pattern F opens this front.
    //
    // n_extra is split: A=15%, B=15%, E=30%, F=40% (F gets the largest
    // share because its profile is best). target_A/B/E reduced accordingly.
    {
        // Pre-compute IP=8 pairs in base. Cap at first M pairs to bound memory.
        const int target_F = (n_extra * 40 + 99) / 100;  // 40% of n_extra
        std::vector<std::pair<int,int>> ip8_pairs;
        ip8_pairs.reserve(target_F * 10);
        for (int i = 0; i < (int)base.size(); i++) {
            for (int j = i + 1; j < (int)base.size(); j++) {
                double d = ip(base[i], base[j]);
                if (d > 8.0 - EPS && d < 8.0 + EPS) {
                    ip8_pairs.push_back({i, j});
                    if ((int)ip8_pairs.size() >= target_F * 6) goto pairs_done;
                }
            }
        }
        pairs_done:;
        std::shuffle(ip8_pairs.begin(), ip8_pairs.end(), rng);

        int gen_F = 0;
        for (size_t pp = 0; pp < ip8_pairs.size() && gen_F < target_F; pp++) {
            int i = ip8_pairs[pp].first;
            int j = ip8_pairs[pp].second;
            // both directions u = v - w and u = w - v
            Vec u1, u2;
            for (int k = 0; k < DIM; k++) {
                u1.c[k] = base[i].c[k] - base[j].c[k];
                u2.c[k] = base[j].c[k] - base[i].c[k];
            }
            // Sanity check norm²; skip if drift (should never happen with
            // exact arithmetic on Z[√3] but EPS-tolerant guard)
            if (std::abs(u1.norm2() - NORM2) < EPS) {
                if (try_insert(u1)) { gen_F++; if (gen_F >= target_F) break; }
            }
            if (std::abs(u2.norm2() - NORM2) < EPS) {
                if (try_insert(u2)) { gen_F++; if (gen_F >= target_F) break; }
            }
        }

        fprintf(stdout, "[POOL_EXT] generated A=%d (target %d), B=%d (target %d), "
                "E=%d (target %d), F=%d (target %d, %zu pairs available)\n",
                gen_A, target_A, gen_B, target_B, gen_E, target_E,
                gen_F, target_F, ip8_pairs.size());
        return out;
    }
}

// (Pool extension function returns above with Pattern F)

// =====================================================================
// Conflict graph (incremental, FIX-B8 style)
//
// State: configuration S = subset of pool of size N_TARGET = 1155.
//        idx_in_S[poolidx] = position in S, or -1 if not in S.
//        S[k] = poolidx.
// Cost:  number of pairs (i,j) in S with ip(pool[S[i]], pool[S[j]]) > IP_THRESH.
// Per-vertex conflict count: viol_count[k] = how many other elements of S
//        are in conflict with S[k]. cost = (sum of viol_count) / 2.
// Updates: O(N_TARGET) per single-replace move (recompute viol_count
//          for the swapped vertex against all others).
// =====================================================================
struct State {
    std::vector<Vec>* pool;        // shared pointer to pool
    std::vector<int> S;            // size N_TARGET; S[k] = pool index
    std::vector<int> idx_in_S;     // idx_in_S[poolidx] = k or -1
    std::vector<int> viol_count;   // size N_TARGET; viol_count[k] = #conflicts
    long long total_viol;          // cost = total_viol (NOT halved -- direct)
    long long pair_violations;     // cost = pair_violations = total_viol / 2

    // V2 audit fix O3: cached hint to highest-violation index. May be stale
    // after some moves; periodically refreshed by Move P2 itself or
    // recompute_full(). When stale, Move P2 re-samples (no correctness risk).
    int max_viol_k_hint = 0;

    // Initialize with given list of pool indices (size must == N_TARGET)
    void init(std::vector<Vec>* pool_, const std::vector<int>& seed_indices) {
        pool = pool_;
        S = seed_indices;
        if ((int)S.size() != N_TARGET) {
            fprintf(stderr, "[FATAL] State::init seed size %zu != N_TARGET %zu\n",
                    S.size(), (size_t)N_TARGET);
            exit(1);
        }
        idx_in_S.assign(pool->size(), -1);
        for (int k = 0; k < N_TARGET; k++) idx_in_S[S[k]] = k;
        viol_count.assign(N_TARGET, 0);
        recompute_full();
    }

    // Recompute viol_count from scratch. O(N²). Used at init + sanity.
    void recompute_full() {
        for (int k = 0; k < N_TARGET; k++) viol_count[k] = 0;
        long long total = 0;
        for (int i = 0; i < N_TARGET; i++) {
            const Vec& vi = (*pool)[S[i]];
            for (int j = i+1; j < N_TARGET; j++) {
                const Vec& vj = (*pool)[S[j]];
                if (ip(vi, vj) > IP_THRESH + EPS) {
                    viol_count[i]++;
                    viol_count[j]++;
                    total += 2;
                }
            }
        }
        total_viol      = total;
        pair_violations = total / 2;
        // V2 audit fix O3: refresh hint after full recompute
        int max_k = 0;
        for (int k = 1; k < N_TARGET; k++) {
            if (viol_count[k] > viol_count[max_k]) max_k = k;
        }
        max_viol_k_hint = max_k;
    }

    // Compute change in cost from replacing S[k] with new pool index nk.
    // Returns delta in pair_violations (signed).
    long long delta_replace(int k, int nk) const {
        if (idx_in_S[nk] != -1) return 0; // already in S; not a valid move
        const Vec& vnew = (*pool)[nk];
        const Vec& vold = (*pool)[S[k]];
        long long old_v = 0;
        long long new_v = 0;
        for (int i = 0; i < N_TARGET; i++) {
            if (i == k) continue;
            const Vec& vi = (*pool)[S[i]];
            if (ip(vi, vold) > IP_THRESH + EPS) old_v++;
            if (ip(vi, vnew) > IP_THRESH + EPS) new_v++;
        }
        return new_v - old_v;
    }

    // Apply replacement S[k] <- nk. Must be a non-stale move (nk not in S).
    void apply_replace(int k, int nk) {
        const Vec& vnew = (*pool)[nk];
        const Vec& vold = (*pool)[S[k]];
        // Update viol_count[i] for i != k. Each pair (i, k) that appears/
        // disappears contributes ±1 to viol_count[i] and the SAME ±1 to
        // viol_count[k]. We accumulate the per-pair change once here.
        long long delta_pairs = 0;  // signed change in number of (i,k) conflict pairs
        for (int i = 0; i < N_TARGET; i++) {
            if (i == k) continue;
            const Vec& vi = (*pool)[S[i]];
            int old_c = (ip(vi, vold) > IP_THRESH + EPS) ? 1 : 0;
            int new_c = (ip(vi, vnew) > IP_THRESH + EPS) ? 1 : 0;
            int d = new_c - old_c;
            if (d != 0) {
                viol_count[i] += d;
                delta_pairs   += d;
            }
        }
        // Apply matching change to viol_count[k]: it equals delta_pairs by
        // construction (every pair (i,k) modified contributes equally to k's
        // count and i's count).
        viol_count[k] += delta_pairs;
        // Sanity: viol_count[k] now matches recomputation from scratch.
        // total_viol = sum_i viol_count[i] = 2 * pair_violations.
        // delta in total_viol = 2 * delta_pairs.
        total_viol      += 2 * delta_pairs;
        pair_violations  = total_viol / 2;
        // commit pool index swap
        idx_in_S[S[k]] = -1;
        S[k] = nk;
        idx_in_S[nk] = k;
    }

    // Compute Δ for atomic dual replace: S[k1]<-nk1, S[k2]<-nk2.
    // k1 != k2; nk1 != nk2; both new indices must not currently be in S.
    long long delta_dual_replace(int k1, int nk1, int k2, int nk2) const {
        if (k1 == k2) return 0;
        if (nk1 == nk2) return 0;
        if (idx_in_S[nk1] != -1 && idx_in_S[nk1] != k1) return 0;
        if (idx_in_S[nk2] != -1 && idx_in_S[nk2] != k2) return 0;
        const Vec& vo1 = (*pool)[S[k1]];
        const Vec& vo2 = (*pool)[S[k2]];
        const Vec& vn1 = (*pool)[nk1];
        const Vec& vn2 = (*pool)[nk2];
        long long old_v = 0;
        long long new_v = 0;
        // pairs involving k1 or k2 with everyone else (excluding k1-k2 itself)
        for (int i = 0; i < N_TARGET; i++) {
            if (i == k1 || i == k2) continue;
            const Vec& vi = (*pool)[S[i]];
            if (ip(vi, vo1) > IP_THRESH + EPS) old_v++;
            if (ip(vi, vn1) > IP_THRESH + EPS) new_v++;
            if (ip(vi, vo2) > IP_THRESH + EPS) old_v++;
            if (ip(vi, vn2) > IP_THRESH + EPS) new_v++;
        }
        // pair (k1, k2)
        if (ip(vo1, vo2) > IP_THRESH + EPS) old_v++;
        if (ip(vn1, vn2) > IP_THRESH + EPS) new_v++;
        return new_v - old_v;
    }

    void apply_dual_replace(int k1, int nk1, int k2, int nk2) {
        // simplest correct implementation: do the two single replaces in
        // sequence; viol_count tracking remains consistent.
        apply_replace(k1, nk1);
        apply_replace(k2, nk2);
    }
};

// =====================================================================
// Cold seeds: two modes
//   (A) ZE99+1 warm-pivot: indices [0..1153] + 1 extension index
//   (B) cross-radical random: 1155 random indices from extended pool
// =====================================================================
static std::vector<int> seed_ze99_plus_one(
    const std::vector<Vec>& pool_extended,
    int ze99_size,
    std::mt19937_64& rng)
{
    std::vector<int> idx;
    idx.reserve(N_TARGET);
    for (int i = 0; i < ze99_size; i++) idx.push_back(i);
    // pick 1 extension index uniformly
    if ((int)pool_extended.size() <= ze99_size) {
        fprintf(stderr, "[FATAL] pool extension has no extra elements\n");
        exit(1);
    }
    std::uniform_int_distribution<int> ext_dist(ze99_size, (int)pool_extended.size()-1);
    idx.push_back(ext_dist(rng));
    return idx;
}

static std::vector<int> seed_random(
    const std::vector<Vec>& pool_extended,
    std::mt19937_64& rng)
{
    int P = (int)pool_extended.size();
    if (P < N_TARGET) {
        fprintf(stderr, "[FATAL] pool too small for random seed: %d < %d\n",
                P, N_TARGET);
        exit(1);
    }
    std::vector<int> all(P);
    for (int i = 0; i < P; i++) all[i] = i;
    std::shuffle(all.begin(), all.end(), rng);
    std::vector<int> idx(all.begin(), all.begin() + N_TARGET);
    return idx;
}

// =====================================================================
// Bounded greedy cleanup for cold seeds (V2 — post-audit fix Aguero #2)
//
// AUDIT FINDING (Rompehuevos): SEED_RANDOM gives initial pair_violations
// ~8050. Engine budget eaten on bulk descent, never reaches plateau where
// Move P1/Q matters (Sobol-F55 spirit).
//
// FIX: bounded greedy 1-pass cleanup. For each k in S, find best pool
// alternative AS LONG AS cost_now > target_band. Stops when target band
// reached OR no improvement on a full pass. PRESERVES cold-start identity
// (no ZE99 pivot enforced) but brings pair_violations into the regime
// where vocabulary primitives matter.
//
// MODULATION OVER AUDITOR PROPOSAL: auditor suggested unlimited 1-pass
// greedy, which can collapse cold identity to local minimum near ZE99
// neighborhood. Bounded version stops at target_band ~= 200-500, leaving
// room for SA to do real exploration.
// =====================================================================
static long long bounded_greedy_cleanup(
    State& st,
    const std::vector<Vec>& pool,
    long long target_band,
    int max_passes)
{
    long long P = (long long)pool.size();
    long long total_gain = 0;
    int passes = 0;
    while (passes < max_passes) {
        if (st.pair_violations <= target_band) break;
        long long pass_gain = 0;
        for (int k = 0; k < N_TARGET; k++) {
            if (st.pair_violations <= target_band) break;
            // Find best replacement for vertex k
            int best_nk = -1;
            long long best_d = 0;
            // sample 256 random pool indices instead of scanning all P
            // (full scan is 5730 × 1155 work per pass; 256 random is much faster
            //  and statistically captures most-improving alternative for cold init)
            std::mt19937_64 local_rng((uint64_t)k * 0xC0FFEELL + (uint64_t)passes);
            std::uniform_int_distribution<int> p_d(0, (int)P - 1);
            for (int t = 0; t < 256; t++) {
                int p = p_d(local_rng);
                if (st.idx_in_S[p] != -1) continue;
                long long d = st.delta_replace(k, p);
                if (d < best_d) {
                    best_d = d;
                    best_nk = p;
                }
            }
            if (best_nk >= 0 && best_d < 0) {
                st.apply_replace(k, best_nk);
                pass_gain   += best_d;
                total_gain  += best_d;
            }
        }
        passes++;
        if (pass_gain == 0) break;
    }
    return total_gain;
}

// =====================================================================
// Move primitives (chain-aware Metropolis)
// =====================================================================
struct MoveStats {
    long long tries    = 0;
    long long accepted = 0;
    long long impr     = 0;
    double acc_rate() const {
        return tries == 0 ? 0.0 : (double)accepted / (double)tries;
    }
};

struct MoveMix {
    int S_, P1_, P2_, Q_, K_, R_;  // weights, sum normally = 100
    int sum() const { return S_+P1_+P2_+Q_+K_+R_; }
    int pick(std::mt19937_64& rng) const {
        std::uniform_int_distribution<int> d(0, sum()-1);
        int x = d(rng);
        if (x < S_)  return 0;
        x -= S_;
        if (x < P1_) return 1;
        x -= P1_;
        if (x < P2_) return 2;
        x -= P2_;
        if (x < Q_)  return 3;
        x -= Q_;
        if (x < K_)  return 4;
        x -= K_;
        return 5;
    }
};

// Chain-state bundle
struct Chain {
    State state;
    long long best_viol;        // best pair_violations seen by this chain
    std::vector<int> best_S;    // best configuration cached
    MoveMix mix;
    MoveStats st_S, st_P1, st_P2, st_Q, st_K, st_R;
    int reheats_count = 0;
    long long last_impr_iter = 0;  // V2 audit fix W3: was int, latent overflow
    int chain_id;
    double T_base;
    int tabu_cap;
    // simple tabu: queue of "forbidden replace" pairs (poolidx)
    std::vector<int> tabu_q;
    int tabu_head = 0;
    bool in_tabu(int poolidx) const {
        for (int i = tabu_head; i < (int)tabu_q.size(); i++) {
            if (tabu_q[i] == poolidx) return true;
        }
        return false;
    }
    void push_tabu(int poolidx) {
        tabu_q.push_back(poolidx);
        if ((int)(tabu_q.size() - tabu_head) > tabu_cap) {
            tabu_head++;
        }
        if (tabu_head > 100000) {
            // compact
            tabu_q.erase(tabu_q.begin(), tabu_q.begin() + tabu_head);
            tabu_head = 0;
        }
    }
};

// =====================================================================
// Single SA iteration on one chain
// Returns: accepted (true) or rejected
// =====================================================================
static bool sa_step(Chain& ch, const std::vector<Vec>& pool,
                    const VecIndexMap& pool_index,
                    std::mt19937_64& rng, long long iter,
                    double T_mod = 1.0)
{
    int move_type = ch.mix.pick(rng);
    long long P = (long long)pool.size();
    double T_eff = ch.T_base * T_mod;

    auto metropolis = [&](long long delta) -> bool {
        if (delta <= 0) return true;
        double e = std::exp(-(double)delta / T_eff);
        std::uniform_real_distribution<double> u(0.0, 1.0);
        return u(rng) < e;
    };

    std::uniform_int_distribution<int> k_dist(0, N_TARGET - 1);
    std::uniform_int_distribution<int> p_dist(0, (int)P - 1);

    if (move_type == 0) {
        // Move S: single replace
        ch.st_S.tries++;
        int k  = k_dist(rng);
        int nk = p_dist(rng);
        if (ch.state.idx_in_S[nk] != -1) return false;
        if (ch.in_tabu(nk)) return false;
        long long delta = ch.state.delta_replace(k, nk);
        if (metropolis(delta)) {
            int old_idx = ch.state.S[k];
            ch.state.apply_replace(k, nk);
            ch.push_tabu(old_idx);
            ch.st_S.accepted++;
            if (delta < 0) ch.st_S.impr++;
            return true;
        }
        return false;
    }
    if (move_type == 1) {
        // Move P1 V3 — conflict-partner targeting (vocabulary virgen real).
        //
        // The v2 P1 picked k1, k2, nk1, nk2 all uniformly random, making P1
        // statistically indistinguishable from two independent Move S calls
        // wrapped in a single Metropolis. That is NOT virgin vocabulary —
        // it's just a slower Move S.
        //
        // V3 P1 atomic dual targets a CONFLICT PAIR:
        //   k1 = a position with non-zero violations (chosen by max_viol_k_hint
        //        or sample-fallback);
        //   k2 = a position that is IN CONFLICT with k1 (one of the violators
        //        of k1 within current S). This is the structural identity:
        //        we are explicitly trying to RESOLVE a conflict by replacing
        //        BOTH endpoints simultaneously.
        //   nk1, nk2 = random pool candidates not in S.
        //
        // The single Metropolis on Δ_combined captures the reasoning:
        //   "Either we resolve this specific conflict together, or we don't."
        // This is the direct kissing analog of Sobol Move P (atomic dual cell
        // with shared Metropolis on combined Δ).
        ch.st_P1.tries++;
        int k1 = ch.state.max_viol_k_hint;
        if (ch.state.viol_count[k1] == 0) {
            // No conflicts visible at hint — sample-fallback then accept
            // we're attacking a near-clean state (still useful in late SA
            // for cohort exploration even without a partner).
            int best_v = 0, best_k = -1;
            for (int t = 0; t < 16; t++) {
                int kc = k_dist(rng);
                if (ch.state.viol_count[kc] > best_v) {
                    best_v = ch.state.viol_count[kc];
                    best_k = kc;
                }
            }
            if (best_k < 0) {
                // truly clean state: degenerate to v2 random P1 (still useful)
                k1 = k_dist(rng);
            } else {
                k1 = best_k;
                ch.state.max_viol_k_hint = k1;
            }
        }
        // Find a conflict partner k2: scan S[i] looking for a violator of k1
        int k2 = -1;
        if (ch.state.viol_count[k1] > 0) {
            // Random offset start to scan-find a partner; expected O(N/viol)
            int start = k_dist(rng);
            const Vec& v1 = (*ch.state.pool)[ch.state.S[k1]];
            for (int off = 0; off < N_TARGET; off++) {
                int i = (start + off) % N_TARGET;
                if (i == k1) continue;
                const Vec& vi = (*ch.state.pool)[ch.state.S[i]];
                if (ip(vi, v1) > IP_THRESH + EPS) {
                    k2 = i;
                    break;
                }
            }
        }
        if (k2 < 0) {
            // Fallback: random partner (preserves coverage)
            k2 = k_dist(rng);
            if (k2 == k1) k2 = (k1 + 1) % N_TARGET;
        }
        int nk1 = p_dist(rng);
        int nk2 = p_dist(rng);
        if (nk1 == nk2) return false;
        if (ch.state.idx_in_S[nk1] != -1) return false;
        if (ch.state.idx_in_S[nk2] != -1) return false;
        long long delta = ch.state.delta_dual_replace(k1, nk1, k2, nk2);
        if (metropolis(delta)) {
            int old1 = ch.state.S[k1], old2 = ch.state.S[k2];
            ch.state.apply_dual_replace(k1, nk1, k2, nk2);
            ch.push_tabu(old1);
            ch.push_tabu(old2);
            ch.st_P1.accepted++;
            if (delta < 0) ch.st_P1.impr++;
            return true;
        }
        return false;
    }
    if (move_type == 2) {
        // Move P2: targeted dual replace. Pick highest-violating S[k1] using
        // cached max_viol_k_hint (V2 audit fix O3); fall back to sample-16
        // if hint is stale (viol_count[hint] == 0).
        ch.st_P2.tries++;
        int k1 = ch.state.max_viol_k_hint;
        if (ch.state.viol_count[k1] == 0) {
            // hint stale; sample-16 fallback to find a high-viol index
            int best_viol = 0;
            int best_k = -1;
            for (int t = 0; t < 16; t++) {
                int kc = k_dist(rng);
                if (ch.state.viol_count[kc] > best_viol) {
                    best_viol = ch.state.viol_count[kc];
                    best_k = kc;
                }
            }
            if (best_k < 0) return false;
            k1 = best_k;
            ch.state.max_viol_k_hint = k1;  // refresh
        }
        int k2 = k_dist(rng);
        if (k2 == k1) return false;
        int nk1 = p_dist(rng);
        int nk2 = p_dist(rng);
        if (nk1 == nk2) return false;
        if (ch.state.idx_in_S[nk1] != -1) return false;
        if (ch.state.idx_in_S[nk2] != -1) return false;
        long long delta = ch.state.delta_dual_replace(k1, nk1, k2, nk2);
        if (metropolis(delta)) {
            int old1 = ch.state.S[k1], old2 = ch.state.S[k2];
            ch.state.apply_dual_replace(k1, nk1, k2, nk2);
            ch.push_tabu(old1);
            ch.push_tabu(old2);
            ch.st_P2.accepted++;
            if (delta < 0) ch.st_P2.impr++;
            return true;
        }
        return false;
    }
    if (move_type == 3) {
        // Move Q: rotational perturbation (V2 — post-audit fix O1)
        // Pick a coordinate axis pair (i,j); apply 90° rotation to S[k];
        // O(1) hash lookup of rotated vector in pool.
        ch.st_Q.tries++;
        int k = k_dist(rng);
        const Vec& vk = pool[ch.state.S[k]];
        std::uniform_int_distribution<int> ax_d(0, DIM-1);
        int i = ax_d(rng);
        int j = ax_d(rng);
        if (i == j) return false;
        // 90° rotation: (a, b) -> (b, -a)
        Vec v_rot = vk;
        double a = vk.c[i], b = vk.c[j];
        v_rot.c[i] = b;
        v_rot.c[j] = -a;
        // O(1) lookup
        auto it = pool_index.find(v_rot);
        if (it == pool_index.end()) return false;
        int found = it->second;
        if (ch.state.idx_in_S[found] != -1) return false;
        if (ch.in_tabu(found)) return false;
        long long delta = ch.state.delta_replace(k, found);
        if (metropolis(delta)) {
            int old_idx = ch.state.S[k];
            ch.state.apply_replace(k, found);
            ch.push_tabu(old_idx);
            ch.st_Q.accepted++;
            if (delta < 0) ch.st_Q.impr++;
            return true;
        }
        return false;
    }
    if (move_type == 4) {
        // Move K: kick. Three random replaces in sequence ignoring metropolis,
        // bounded acceptance: only commit if cumulative delta <= +bound.
        ch.st_K.tries++;
        int kicks = 3;
        std::vector<int> backup_old(kicks);
        std::vector<int> backup_new(kicks);
        std::vector<int> ks(kicks);
        long long acc_delta = 0;
        bool ok = true;
        for (int t = 0; t < kicks; t++) {
            int k  = k_dist(rng);
            int nk = p_dist(rng);
            if (ch.state.idx_in_S[nk] != -1) { ok = false; break; }
            long long d = ch.state.delta_replace(k, nk);
            backup_old[t] = ch.state.S[k];
            backup_new[t] = nk;
            ks[t] = k;
            ch.state.apply_replace(k, nk);
            acc_delta += d;
        }
        if (!ok) return false;
        // Allow if cumulative delta <= 5 (small lift OK for exploration)
        long long bound = 5;
        bool accept = (acc_delta <= bound);
        if (!accept) {
            // unwind in reverse
            for (int t = kicks - 1; t >= 0; t--) {
                ch.state.apply_replace(ks[t], backup_old[t]);
            }
            return false;
        }
        for (int t = 0; t < kicks; t++) ch.push_tabu(backup_old[t]);
        ch.st_K.accepted++;
        if (acc_delta < 0) ch.st_K.impr++;
        return true;
    }
    // move_type == 5: Move R = soft restart toward best_S
    ch.st_R.tries++;
    if (ch.best_S.empty()) return false;
    // copy random subset (50 positions) from best_S into current
    std::uniform_int_distribution<int> n_d(0, N_TARGET-1);
    int hits = 50;
    long long acc = 0;
    std::vector<int> rb_old, rb_k;
    for (int t = 0; t < hits; t++) {
        int k = n_d(rng);
        int target = ch.best_S[k];
        if (ch.state.S[k] == target) continue;
        if (ch.state.idx_in_S[target] != -1) continue;
        rb_old.push_back(ch.state.S[k]);
        rb_k.push_back(k);
        long long d = ch.state.delta_replace(k, target);
        ch.state.apply_replace(k, target);
        acc += d;
    }
    // accept always (R is restart)
    ch.st_R.accepted++;
    if (acc < 0) ch.st_R.impr++;
    (void)iter;
    return true;
}

// =====================================================================
// Triple-rescue (V2 — post-audit fix W1, no longer a stub)
//
// AUDIT FINDING (Rompehuevos): original maybe_triple_rescue was placeholder
// that only updated counters. No actual T modulation. Engine claimed
// triple-rescue and delivered nothing.
//
// V2 implementation: three plateau levels with progressive temperature
// boost for a fixed window of iters, then revert. Tracks plateau distance
// since last_impr_iter and applies bumps proportional to depth.
//
//   Plateau 1 (5000 iters since impr): T_base × 1.20 for 1000 iters
//   Plateau 2 (7000 iters since impr): T_base × 1.45 for 1500 iters
//   Plateau 3 (11000 iters since impr): T_base × 1.80 for 2000 iters
//   Adaptive promote (12500 iters): replace S with best_S + perturb 30 vertices
//
// Boost is one-shot per plateau crossing (avoids continuous reheating).
// =====================================================================
struct PlateauState {
    long long boost_until_iter = -1;   // T modulation active until this iter
    double    T_modulation     = 1.0;  // multiplier on T_base when active
    int       plateaus_fired   = 0;    // 1, 2, or 3 since last improvement
};

static void maybe_triple_rescue(Chain& ch, PlateauState& ps,
                                long long iter, std::mt19937_64& rng,
                                const std::vector<Vec>& pool)
{
    // Reset plateau state on improvement (signaled by recent last_impr_iter)
    int since = (int)(iter - ch.last_impr_iter);
    if (since == 0) {
        ps.plateaus_fired = 0;
        ps.boost_until_iter = -1;
        ps.T_modulation = 1.0;
        return;
    }

    // Expire active boost
    if (iter > ps.boost_until_iter) {
        ps.T_modulation = 1.0;
    }

    if (ch.reheats_count >= MAX_REHEATS) return;

    auto fire_boost = [&](double factor, int duration, const char* label) {
        ps.T_modulation     = factor;
        ps.boost_until_iter = iter + duration;
        ps.plateaus_fired++;
        ch.reheats_count++;
        // Note: actual T_base is read from ch.T_base in sa_step. We modulate
        // by storing the active multiplier in ch.T_base directly here, but
        // need to remember the original. We'll store the un-modulated base
        // separately for clarity (added as ch.T_base_original below).
        fprintf(stdout, "[RESCUE %s chain=%d iter=%lld] "
                "T_base %.4f -> %.4f for %d iters\n",
                label, ch.chain_id, iter,
                ch.T_base, ch.T_base * factor, duration);
        std::fflush(stdout);
    };

    if (since >= 11000 && ps.plateaus_fired < 3) {
        fire_boost(1.80, 2000, "P3");
    } else if (since >= 7000 && ps.plateaus_fired < 2) {
        fire_boost(1.45, 1500, "P2");
    } else if (since >= 5000 && ps.plateaus_fired < 1) {
        fire_boost(1.20, 1000, "P1");
    }

    // Adaptive promote: revert to best_S + perturb 30 random vertices
    if (since >= 12500 && !ch.best_S.empty()) {
        // Hard reset: copy best_S into state, then randomly perturb 30
        // vertices using fresh pool indices.
        // (Implemented via direct apply_replace calls; safe because best_S
        // is a kissing-near config.)
        int total_p = (int)pool.size();
        std::uniform_int_distribution<int> p_d(0, total_p - 1);
        std::uniform_int_distribution<int> k_d(0, N_TARGET - 1);
        // first revert to best_S
        for (int k = 0; k < N_TARGET; k++) {
            if (ch.state.S[k] != ch.best_S[k] &&
                ch.state.idx_in_S[ch.best_S[k]] == -1) {
                ch.state.apply_replace(k, ch.best_S[k]);
            }
        }
        // then perturb 30 random vertices
        int perturbed = 0;
        for (int t = 0; t < 200 && perturbed < 30; t++) {
            int k  = k_d(rng);
            int nk = p_d(rng);
            if (ch.state.idx_in_S[nk] == -1) {
                ch.state.apply_replace(k, nk);
                perturbed++;
            }
        }
        ch.last_impr_iter = iter;  // reset plateau timer
        ps.plateaus_fired = 0;
        fprintf(stdout, "[RESCUE PROMOTE chain=%d iter=%lld] "
                "reverted to best_S + perturbed %d vertices, viol=%lld\n",
                ch.chain_id, iter, perturbed, ch.state.pair_violations);
        std::fflush(stdout);
    }
}

// =====================================================================
// MIX_RECAL V3 — productivity-driven, not acceptance-driven
//
// AUDIT FINDING (Constructor Rompehuevos): at hot T (chain 4 T=1.40),
// Move S acceptance is dominated by Metropolis lottery, not real value.
// A move can have acc_rate=0.40 while delivering 0% gbest improvement —
// pure noise. Conversely Move P1/Q targeting basin escapes have low
// acceptance but every accepted move that improves IS structural gain.
//
// V3 fix: weight bump by IMPROVEMENT RATE (impr/tries), not acceptance.
// This is the Sobol decision-matrix transfer correctly applied to LUNA:
// productive moves earn weight; merely acceptable moves do not.
//
// Window-based: each call resets per-window counters so weights track
// recent productivity, not lifetime average.
// =====================================================================
static void mix_recal(Chain& ch)
{
    auto prod = [](const MoveStats& s) -> double {
        return s.tries == 0 ? 0.0 : (double)s.impr / (double)s.tries;
    };
    double pS  = prod(ch.st_S);
    double pP1 = prod(ch.st_P1);
    double pP2 = prod(ch.st_P2);
    double pQ  = prod(ch.st_Q);
    double pK  = prod(ch.st_K);
    double pR  = prod(ch.st_R);
    // Threshold scaled to typical kissing-13 productivities.
    // Empirically Move S impr-rate at warm T is 1-5%; Move P1 targeted 0.5-2%.
    auto bump = [&](int& w, double p) {
        if (p > 0.020) {                                 // > 2% impr is gold
            w = (int)std::round(w * MIX_RECAL_BUMP_UP);
        } else if (p < 0.002) {                          // < 0.2% impr is dead
            w = (int)std::round(w * MIX_RECAL_BUMP_DN);
        }
        if (w < 1) w = 1;
        if (w > 60) w = 60;                              // V3: cap raised to 60
    };
    bump(ch.mix.S_,  pS);
    bump(ch.mix.P1_, pP1);
    bump(ch.mix.P2_, pP2);
    bump(ch.mix.Q_,  pQ);
    // Move K (kick) and Move R (soft restart) are exploration tools, not
    // direct improvement. Their accept rates are structurally biased (R
    // always-accept, K rarely-accept) and bumping by productivity gives
    // misleading weights. Keep them at fixed exploration baseline (10).
    ch.mix.K_ = 10;
    ch.mix.R_ = 10;
    (void)pK; (void)pR;  // logged but not used for bump
}

// =====================================================================
// Parallel Tempering swap between adjacent chains
// =====================================================================
static int pt_swap(std::vector<Chain>& chains, std::mt19937_64& rng)
{
    int swaps = 0;
    for (int i = 0; i < (int)chains.size() - 1; i++) {
        long long c1 = chains[i].state.pair_violations;
        long long c2 = chains[i+1].state.pair_violations;
        long long dE = c2 - c1;
        double T1 = chains[i].T_base;
        double T2 = chains[i+1].T_base;
        double arg = (1.0/T1 - 1.0/T2) * (double)dE;
        double p = (arg >= 0) ? 1.0 : std::exp(arg);
        std::uniform_real_distribution<double> u(0.0, 1.0);
        if (u(rng) < p) {
            std::swap(chains[i].state, chains[i+1].state);
            swaps++;
        }
    }
    return swaps;
}

// =====================================================================
// Polish pass: greedy single-replace exhaustive scan, accept any delta < 0
// =====================================================================
static long long polish_once(State& st, const std::vector<Vec>& pool)
{
    long long P = (long long)pool.size();
    long long total_gain = 0;
    bool any = true;
    int passes = 0;
    while (any && passes < POLISH_PASSES_MAX) {
        any = false;
        for (int k = 0; k < N_TARGET; k++) {
            // try every pool index
            int best_nk = -1;
            long long best_d = 0;
            for (long long p = 0; p < P; p++) {
                if (st.idx_in_S[(int)p] != -1) continue;
                long long d = st.delta_replace(k, (int)p);
                if (d < best_d) {
                    best_d = d;
                    best_nk = (int)p;
                }
            }
            if (best_nk >= 0 && best_d < 0) {
                st.apply_replace(k, best_nk);
                total_gain += best_d;
                any = true;
            }
        }
        passes++;
    }
    return total_gain;
}

// =====================================================================
// D190 verification: dump configuration to file, format compatible with
// independent Python kernel re-audit.
// =====================================================================
static void dump_config(const State& st, const std::vector<Vec>& pool,
                        const char* path, long long viol)
{
    std::ofstream fout(path);
    if (!fout) {
        fprintf(stderr, "[WARN] cannot open dump path %s\n", path);
        return;
    }
    fout << "# DOSPIRATAS dump | size=" << N_TARGET
         << " | pair_violations=" << viol << "\n";
    for (int k = 0; k < N_TARGET; k++) {
        const Vec& v = pool[st.S[k]];
        for (int i = 0; i < DIM; i++) {
            if (i > 0) fout << ",";
            fout << v.c[i];
        }
        fout << "\n";
    }
    fout.close();
}

// =====================================================================
// In-engine final verify (paso 1 of D190): recompute cost from scratch
// =====================================================================
static long long verify_cost_from_scratch(const State& st,
                                          const std::vector<Vec>& pool)
{
    long long count = 0;
    for (int i = 0; i < N_TARGET; i++) {
        for (int j = i+1; j < N_TARGET; j++) {
            if (ip(pool[st.S[i]], pool[st.S[j]]) > IP_THRESH + EPS) count++;
        }
    }
    return count;
}

// =====================================================================
// Sanity suite
// =====================================================================
static bool sanity_suite(const std::vector<Vec>& pool, const State& st)
{
    bool ok = true;
    // #1: pool norms² == 16
    for (size_t i = 0; i < pool.size(); i++) {
        if (std::abs(pool[i].norm2() - NORM2) > EPS) {
            fprintf(stderr, "[SANITY-1] FAIL pool[%zu] norm²=%.6f\n",
                    i, pool[i].norm2());
            ok = false;
            break;
        }
    }
    // #2: state size == N_TARGET
    if ((int)st.S.size() != N_TARGET) {
        fprintf(stderr, "[SANITY-2] FAIL state size %zu\n", st.S.size());
        ok = false;
    }
    // #3: no duplicates in S
    std::unordered_set<int> seen;
    for (int k = 0; k < N_TARGET; k++) {
        if (!seen.insert(st.S[k]).second) {
            fprintf(stderr, "[SANITY-3] FAIL dup pool index in S: %d\n", st.S[k]);
            ok = false;
            break;
        }
    }
    // #4: idx_in_S consistent
    for (int k = 0; k < N_TARGET; k++) {
        if (st.idx_in_S[st.S[k]] != k) {
            fprintf(stderr, "[SANITY-4] FAIL idx_in_S inconsistent at k=%d\n", k);
            ok = false;
            break;
        }
    }
    return ok;
}

// =====================================================================
// Main
// =====================================================================
int main(int argc, char** argv)
{
    fprintf(stdout,
        "==========================================================\n"
        "  DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS\n"
        "  LUNA project, K(13) >= %d cold-start SA attempt\n"
        "  Sobol-F55 inspired, virgin Move P/Q vocabulary\n"
        "  Mac M2 25%% CPU, 5-chain PT, ~4-8h wall-clock budget\n"
        "==========================================================\n",
        N_TARGET);
    std::fflush(stdout);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pool_file> [--seed-mode=ze99|random|alternating]\n",
                argv[0]);
        return 1;
    }
    const char* pool_path = argv[1];
    enum SeedMode { SEED_ZE99, SEED_RANDOM, SEED_ALT } seed_mode = SEED_ALT;  // V2 default per audit
    for (int a = 2; a < argc; a++) {
        std::string arg = argv[a];
        if (arg == "--seed-mode=ze99")        seed_mode = SEED_ZE99;
        else if (arg == "--seed-mode=random") seed_mode = SEED_RANDOM;
        else if (arg == "--seed-mode=alternating") seed_mode = SEED_ALT;
        else {
            fprintf(stderr, "[FATAL] unknown arg: %s\n", argv[a]);
            return 1;
        }
    }
    const char* seed_mode_name = (seed_mode == SEED_ZE99) ? "ZE99+1 warm" :
                                  (seed_mode == SEED_RANDOM) ? "random cold (with bounded cleanup)" :
                                  "alternating (default V2)";
    fprintf(stdout, "[CONFIG] seed_mode=%s\n", seed_mode_name);

    std::mt19937_64 rng(RNG_SEED);

    // 1. Load ZE99 pool
    std::vector<Vec> pool_base = load_pool(pool_path);
    fprintf(stdout, "[LOAD] base pool: %zu vectors loaded from %s\n",
            pool_base.size(), pool_path);

    // 2. Extend with cross-radical candidates
    // V3 record-attack: pool extension boosted from 4000 to 8000.
    //   A=1200, B=1200, E=2400, F=3200 (Pattern F is the structural prize).
    //   Memory: 8000 vectors × 13 doubles × 8 bytes = 832 KB. Trivial.
    int N_EXTRA = 8000;
    std::vector<Vec> pool = extend_pool_cross_radical(pool_base, rng, N_EXTRA);
    fprintf(stdout, "[POOL] extended pool size: %zu (base %zu + ~%d extra)\n",
            pool.size(), pool_base.size(), N_EXTRA);

    // 2b. Build pool_index for O(1) Move Q lookup (V2 audit fix O1)
    VecIndexMap pool_index;
    pool_index.reserve(pool.size() * 2);
    for (int p = 0; p < (int)pool.size(); p++) {
        pool_index[pool[p]] = p;
    }
    fprintf(stdout, "[POOL_INDEX] hash map built, %zu entries\n",
            pool_index.size());

    // 3. Initialize 5 chains per seed_mode
    std::vector<Chain> chains(N_CHAINS);
    int ze99_size = (int)pool_base.size();
    for (int c = 0; c < N_CHAINS; c++) {
        chains[c].chain_id = c;
        chains[c].T_base   = T_LADDER[c];
        chains[c].tabu_cap = TABU_CAPS[c];
        std::vector<int> seed;
        SeedMode this_mode = seed_mode;
        if (seed_mode == SEED_ALT) {
            this_mode = (c % 2 == 0) ? SEED_ZE99 : SEED_RANDOM;
        }
        if (this_mode == SEED_ZE99) {
            seed = seed_ze99_plus_one(pool, ze99_size, rng);
            chains[c].state.init(&pool, seed);
        } else {
            seed = seed_random(pool, rng);
            chains[c].state.init(&pool, seed);
            // V2 audit fix Aguero #2: bounded greedy cleanup brings cold init
            // into descent regime where vocabulary primitives matter.
            long long pre  = chains[c].state.pair_violations;
            long long target_band = 300;
            int max_passes = 4;
            long long gain = bounded_greedy_cleanup(chains[c].state, pool,
                                                    target_band, max_passes);
            long long post = chains[c].state.pair_violations;
            fprintf(stdout, "[INIT_CLEANUP chain=%d cold] pre=%lld -> post=%lld "
                    "(gain=%lld, target_band=%lld)\n",
                    c, pre, post, gain, target_band);
        }
        chains[c].best_viol = chains[c].state.pair_violations;
        chains[c].best_S    = chains[c].state.S;
        // V3 cost-probe pre-SA: Sobol decision matrix transfer.
        // Initial mix is determined by EMPIRICAL productivity probe of each
        // move on this chain's actual state, not by uniform balanced default.
        //
        // For each move type m, run F2_TRIALS independent probes from the
        // initial state (rolled back each time). Measure improvement rate
        // (impr/tries). Convert to weight via thresholds:
        //   p < F2_ORNAMENT (0.001): weight 2 (ornamental, kept for coverage)
        //   p < F2_PROBE    (0.005): weight 8 (probe-mass)
        //   p >= F2_PROBE          : weight 30 (full hammer)
        // Move K and Move R always get baseline weight (10) regardless —
        // they are exploration tools, not direct improvement tools.
        chains[c].mix = MoveMix{30, 20, 15, 10, 15, 10};  // fallback default
    }

    // 3b. Pre-SA cost-probe: measure each chain's per-move productivity
    //     on its actual init state, then update mix accordingly.
    fprintf(stdout, "[COST_PROBE] running per-move productivity probes "
            "(%d trials each, T_probe=%.2f)...\n", F1_TRIALS, T_PROBE_F2);
    for (int c = 0; c < N_CHAINS; c++) {
        struct ProbeStats { int tries=0, impr=0; };
        ProbeStats ps_S, ps_P1, ps_P2, ps_Q;

        // Save state for rollback
        long long saved_viol = chains[c].state.pair_violations;
        std::vector<int> saved_S = chains[c].state.S;
        std::vector<int> saved_idx = chains[c].state.idx_in_S;
        std::vector<int> saved_vc = chains[c].state.viol_count;
        long long saved_total = chains[c].state.total_viol;
        int saved_hint = chains[c].state.max_viol_k_hint;

        auto rollback = [&]() {
            chains[c].state.S = saved_S;
            chains[c].state.idx_in_S = saved_idx;
            chains[c].state.viol_count = saved_vc;
            chains[c].state.total_viol = saved_total;
            chains[c].state.pair_violations = saved_viol;
            chains[c].state.max_viol_k_hint = saved_hint;
        };

        // Force a temporary high-T_mod for probe (capture productivity at probe T)
        double T_save = chains[c].T_base;
        chains[c].T_base = T_PROBE_F2;

        // Reset move stats for clean probe count
        MoveStats sS = chains[c].st_S, sP1 = chains[c].st_P1;
        MoveStats sP2 = chains[c].st_P2, sQ = chains[c].st_Q;
        chains[c].st_S = MoveStats{};
        chains[c].st_P1 = MoveStats{};
        chains[c].st_P2 = MoveStats{};
        chains[c].st_Q = MoveStats{};

        // Probe each move type. Force the move via temp mix that picks only that move.
        MoveMix saved_mix = chains[c].mix;
        auto probe_one = [&](int move_idx, ProbeStats& st) {
            chains[c].mix = MoveMix{0,0,0,0,0,0};
            switch (move_idx) {
              case 0: chains[c].mix.S_  = 100; break;
              case 1: chains[c].mix.P1_ = 100; break;
              case 2: chains[c].mix.P2_ = 100; break;
              case 3: chains[c].mix.Q_  = 100; break;
            }
            for (int t = 0; t < F2_TRIALS; t++) {
                long long pre = chains[c].state.pair_violations;
                sa_step(chains[c], pool, pool_index, rng, t, 1.0);
                long long post = chains[c].state.pair_violations;
                st.tries++;
                if (post < pre) st.impr++;
                rollback();
            }
        };
        probe_one(0, ps_S);
        probe_one(1, ps_P1);
        probe_one(2, ps_P2);
        probe_one(3, ps_Q);

        // Restore stats and T
        chains[c].mix = saved_mix;
        chains[c].T_base = T_save;
        chains[c].st_S = sS; chains[c].st_P1 = sP1;
        chains[c].st_P2 = sP2; chains[c].st_Q = sQ;

        auto rate = [](const ProbeStats& s) -> double {
            return s.tries == 0 ? 0.0 : (double)s.impr / (double)s.tries;
        };
        double pS = rate(ps_S), pP1 = rate(ps_P1);
        double pP2 = rate(ps_P2), pQ = rate(ps_Q);

        auto pick_w = [](double p) -> int {
            if (p >= F2_PROBE)    return 30;
            if (p >= F2_ORNAMENT) return 8;
            return 2;
        };
        chains[c].mix.S_  = pick_w(pS);
        chains[c].mix.P1_ = pick_w(pP1);
        chains[c].mix.P2_ = pick_w(pP2);
        chains[c].mix.Q_  = pick_w(pQ);
        chains[c].mix.K_  = 10;
        chains[c].mix.R_  = 10;

        fprintf(stdout, "[COST_PROBE chain=%d] "
                "S=%.3f%% P1=%.3f%% P2=%.3f%% Q=%.3f%% -> "
                "mix S=%d P1=%d P2=%d Q=%d K=%d R=%d\n",
                c, pS*100, pP1*100, pP2*100, pQ*100,
                chains[c].mix.S_, chains[c].mix.P1_,
                chains[c].mix.P2_, chains[c].mix.Q_,
                chains[c].mix.K_, chains[c].mix.R_);
    }
    std::fflush(stdout);

    // 4. Sanity suite
    fprintf(stdout, "[SANITY] running pre-SA suite...\n");
    if (!sanity_suite(pool, chains[0].state)) {
        fprintf(stderr, "[FATAL] sanity suite failed\n");
        return 1;
    }
    fprintf(stdout, "[SANITY] all 4 tests PASS\n");

    // 5. Print initial cost per chain
    long long initial_min = LLONG_MAX;
    for (int c = 0; c < N_CHAINS; c++) {
        long long v = chains[c].state.pair_violations;
        if (v < initial_min) initial_min = v;
        fprintf(stdout, "[INIT] chain %d T=%.3f pair_violations=%lld\n",
                c, chains[c].T_base, v);
    }
    fprintf(stdout, "[INIT] best chain: pair_violations=%lld (target=0 for record)\n",
            initial_min);
    std::fflush(stdout);

    // 6. SA main loop with PT + triple-rescue + MIX_RECAL + auto-extend
    auto t_start = std::chrono::steady_clock::now();

    long long iter = 0;
    long long iters_budget = ITERS_PER_CHAIN;
    int extensions = 0;
    long long last_recal_iter = 0;
    long long last_log_iter   = 0;
    long long pt_swaps_total  = 0;

    long long gbest_viol = LLONG_MAX;
    std::vector<int> gbest_S;

    auto log_status = [&](long long it) {
        long long min_v = LLONG_MAX;
        int min_c = -1;
        for (int c = 0; c < N_CHAINS; c++) {
            if (chains[c].best_viol < min_v) {
                min_v = chains[c].best_viol;
                min_c = c;
            }
        }
        if (min_v < gbest_viol) {
            gbest_viol = min_v;
            gbest_S    = chains[min_c].best_S;
        }
        auto t_now = std::chrono::steady_clock::now();
        double secs = std::chrono::duration<double>(t_now - t_start).count();
        fprintf(stdout, "[ITER %10lld] gbest=%lld | chain_bests=[",
                it, gbest_viol);
        for (int c = 0; c < N_CHAINS; c++) {
            fprintf(stdout, " %lld", chains[c].best_viol);
        }
        fprintf(stdout, " ] | PT_swaps=%lld | t=%.1fs\n",
                pt_swaps_total, secs);
        std::fflush(stdout);
    };

    bool record_hit = false;

    // V2 audit fix W1: per-chain plateau state for actual triple-rescue
    std::vector<PlateauState> plateau_states(N_CHAINS);

    // Bookkeeping integrity check: recompute from scratch and verify
    // incremental tracking matches. Fires periodically + before record claim.
    auto integrity_check = [&](int chain_id, long long it) -> bool {
        long long claimed = chains[chain_id].state.pair_violations;
        long long fresh   = verify_cost_from_scratch(chains[chain_id].state, pool);
        if (claimed != fresh) {
            fprintf(stderr, "[INTEGRITY-FAIL chain=%d iter=%lld] "
                    "claimed=%lld fresh=%lld DIFF=%lld\n",
                    chain_id, it, claimed, fresh, fresh - claimed);
            return false;
        }
        return true;
    };

    while (iter < iters_budget) {
        // Step every chain once
        for (int c = 0; c < N_CHAINS; c++) {
            // V2 audit fix W1: apply plateau-driven T modulation
            double T_mod = (iter <= plateau_states[c].boost_until_iter)
                         ? plateau_states[c].T_modulation : 1.0;
            sa_step(chains[c], pool, pool_index, rng, iter, T_mod);
            // Track best
            long long pv = chains[c].state.pair_violations;
            if (pv < chains[c].best_viol) {
                chains[c].best_viol     = pv;
                chains[c].best_S        = chains[c].state.S;
                chains[c].last_impr_iter = iter;
                if (pv == 0) {
                    // CRITICAL: verify before claiming record candidate
                    long long fresh = verify_cost_from_scratch(chains[c].state, pool);
                    if (fresh == 0) {
                        record_hit = true;
                    } else {
                        fprintf(stderr, "[FALSE-POSITIVE chain=%d iter=%lld] "
                                "pv=0 but fresh=%lld; bookkeeping bug, forcing recompute\n",
                                c, iter, fresh);
                        chains[c].state.recompute_full();
                        chains[c].best_viol = chains[c].state.pair_violations;
                    }
                }
            }
            maybe_triple_rescue(chains[c], plateau_states[c], iter, rng, pool);
        }

        // PT swap every PT_SWAP_INTERVAL
        if (iter > 0 && iter % PT_SWAP_INTERVAL == 0) {
            pt_swaps_total += pt_swap(chains, rng);
        }

        // MIX_RECAL
        if (iter - last_recal_iter >= MIX_RECAL_ITER) {
            for (int c = 0; c < N_CHAINS; c++) {
                mix_recal(chains[c]);
            }
            last_recal_iter = iter;
        }

        // Periodic integrity check (every 100k iters; expensive but safe)
        if (iter > 0 && iter % 100'000 == 0) {
            for (int c = 0; c < N_CHAINS; c++) {
                if (!integrity_check(c, iter)) {
                    fprintf(stderr, "[RECOVER] forcing recompute_full on chain %d\n", c);
                    chains[c].state.recompute_full();
                    if (chains[c].state.pair_violations < chains[c].best_viol) {
                        chains[c].best_viol = chains[c].state.pair_violations;
                        chains[c].best_S    = chains[c].state.S;
                    }
                }
            }
        }

        // Periodic log every 50,000 iters
        if (iter - last_log_iter >= 50'000) {
            log_status(iter);
            last_log_iter = iter;
        }

        // V3 paper-grade per-move productivity log every 1M iters.
        // This is the data that becomes a paper figure: how acceptance and
        // productivity per move evolve as SA descends through plateaus.
        if (iter > 0 && iter % 1'000'000 == 0) {
            fprintf(stdout, "[PAPER_GRADE iter=%lld]\n", iter);
            for (int c = 0; c < N_CHAINS; c++) {
                const Chain& ch = chains[c];
                auto pct = [](long long n, long long d) -> double {
                    return d == 0 ? 0.0 : 100.0 * (double)n / (double)d;
                };
                fprintf(stdout, "  chain %d T=%.3f viol=%lld best=%lld | "
                        "S:%.2f%%/%.3f%% P1:%.2f%%/%.3f%% P2:%.2f%%/%.3f%% "
                        "Q:%.2f%%/%.3f%% K:%.2f%%/%.3f%% R:%.2f%%/%.3f%% | "
                        "mix=%d/%d/%d/%d/%d/%d\n",
                        c, ch.T_base,
                        ch.state.pair_violations, ch.best_viol,
                        pct(ch.st_S.accepted, ch.st_S.tries),
                        pct(ch.st_S.impr, ch.st_S.tries),
                        pct(ch.st_P1.accepted, ch.st_P1.tries),
                        pct(ch.st_P1.impr, ch.st_P1.tries),
                        pct(ch.st_P2.accepted, ch.st_P2.tries),
                        pct(ch.st_P2.impr, ch.st_P2.tries),
                        pct(ch.st_Q.accepted, ch.st_Q.tries),
                        pct(ch.st_Q.impr, ch.st_Q.tries),
                        pct(ch.st_K.accepted, ch.st_K.tries),
                        pct(ch.st_K.impr, ch.st_K.tries),
                        pct(ch.st_R.accepted, ch.st_R.tries),
                        pct(ch.st_R.impr, ch.st_R.tries),
                        ch.mix.S_, ch.mix.P1_, ch.mix.P2_,
                        ch.mix.Q_, ch.mix.K_, ch.mix.R_);
            }
            std::fflush(stdout);
        }

        if (record_hit) {
            fprintf(stdout, "[!!! RECORD CANDIDATE !!!] gbest_viol == 0 reached "
                    "at iter %lld. Beginning verification.\n", iter);
            std::fflush(stdout);
            break;
        }

        iter++;

        // Auto-extend
        if (iter >= iters_budget && extensions < MAX_EXTENSIONS && !record_hit) {
            // Extend only if at least one chain made progress in last EXTEND_BY iters
            bool progress = false;
            for (int c = 0; c < N_CHAINS; c++) {
                if (iter - chains[c].last_impr_iter < EXTEND_BY) {
                    progress = true; break;
                }
            }
            if (progress) {
                iters_budget += EXTEND_BY;
                extensions++;
                fprintf(stdout, "[AUTO-EXTEND %d/%d] new budget=%lld\n",
                        extensions, MAX_EXTENSIONS, iters_budget);
            } else {
                fprintf(stdout, "[AUTO-EXTEND] no chain progress in last %lld iters; "
                                "stopping extensions\n", EXTEND_BY);
                break;
            }
        }
    }

    log_status(iter);

    // 7. Polish post-SA on best state
    fprintf(stdout, "[POLISH] starting on gbest_viol=%lld\n", gbest_viol);
    std::fflush(stdout);
    State polish_state;
    polish_state.init(&pool, gbest_S);
    long long polish_gain = polish_once(polish_state, pool);
    fprintf(stdout, "[POLISH] gain=%lld | post-polish viol=%lld\n",
            polish_gain, polish_state.pair_violations);
    if (polish_state.pair_violations < gbest_viol) {
        gbest_viol = polish_state.pair_violations;
        gbest_S    = polish_state.S;
    }

    // 8. D190 verification (paso 1: in-engine recompute)
    long long verify_viol = verify_cost_from_scratch(polish_state, pool);
    fprintf(stdout, "[D190 paso 1 FINAL_VERIFY] in-engine recompute=%lld | "
            "claimed=%lld | %s\n",
            verify_viol, gbest_viol,
            (verify_viol == gbest_viol ? "PASS" : "FAIL"));

    // 9. Dump for D190 paso 4 (independent Python verify)
    char dump_path[512];
    snprintf(dump_path, sizeof(dump_path),
             "DOSPIRATAS_dump_viol%lld_size%d.txt",
             gbest_viol, N_TARGET);
    dump_config(polish_state, pool, dump_path, gbest_viol);
    fprintf(stdout, "[DUMP] written to %s\n", dump_path);

    // 10. Final verdict
    fprintf(stdout, "==========================================================\n");
    if (gbest_viol == 0) {
        fprintf(stdout, "*** RECORD CANDIDATE: K(13) >= %d with pair_violations=0 ***\n"
                        "*** SUCCESSOR MUST RUN D190 paso 2/3/4 BEFORE CLAIMING ***\n",
                N_TARGET);
    } else {
        fprintf(stdout, "[CIERRE] best pair_violations=%lld with size=%d\n"
                        "         (record requires pair_violations=0)\n"
                        "         Wall registered: F-dospiratas-1 candidate\n",
                gbest_viol, N_TARGET);
    }
    fprintf(stdout, "==========================================================\n");

    // Per-chain move-stats summary
    for (int c = 0; c < N_CHAINS; c++) {
        const Chain& ch = chains[c];
        fprintf(stdout, "[CHAIN %d T=%.3f] best=%lld | reheats=%d\n"
                "  S : tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  P1: tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  P2: tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  Q : tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  K : tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  R : tries=%lld acc=%lld (%.1f%%) impr=%lld\n"
                "  mix final = S=%d P1=%d P2=%d Q=%d K=%d R=%d\n",
                c, ch.T_base, ch.best_viol, ch.reheats_count,
                ch.st_S.tries,  ch.st_S.accepted,  ch.st_S.acc_rate()*100, ch.st_S.impr,
                ch.st_P1.tries, ch.st_P1.accepted, ch.st_P1.acc_rate()*100, ch.st_P1.impr,
                ch.st_P2.tries, ch.st_P2.accepted, ch.st_P2.acc_rate()*100, ch.st_P2.impr,
                ch.st_Q.tries,  ch.st_Q.accepted,  ch.st_Q.acc_rate()*100, ch.st_Q.impr,
                ch.st_K.tries,  ch.st_K.accepted,  ch.st_K.acc_rate()*100, ch.st_K.impr,
                ch.st_R.tries,  ch.st_R.accepted,  ch.st_R.acc_rate()*100, ch.st_R.impr,
                ch.mix.S_, ch.mix.P1_, ch.mix.P2_, ch.mix.Q_, ch.mix.K_, ch.mix.R_);
    }

    auto t_end = std::chrono::steady_clock::now();
    double total_secs = std::chrono::duration<double>(t_end - t_start).count();
    fprintf(stdout, "[TOTAL WALL-CLOCK] %.1f s = %.2f h\n",
            total_secs, total_secs / 3600.0);
    fprintf(stdout, "[ITERS] %lld iterations | %.0f iters/sec\n",
            iter, iter / std::max(total_secs, 1.0));
    return 0;
}
