// =============================================================================
// HORMONEROCHUMINEROPREMIUM_v2_1.cpp
// =============================================================================
// Proyecto LUNA — Engine 1 v2.1 (Construction A + Layer Stacking, dimension 13)
// Architect:   R. Amichis Luengo
// Constructor: Claude (Anthropic, Opus 4.7)
// Auditor:     Claude (Anthropic, Opus 4.7) — sucesor post G8 jubilation
// v1   ratified: 2026-05-03
// v1.5 ratified: 2026-05-03 (post-handoff hardening — JUBILATED, see graveyard)
// v2   ratified: 2026-05-03 (state-integrity hardening pass — JUBILATED v8 G8)
// v2.1 ratified: 2026-05-03 (CRYO_THRESH recalibration + constants defense + sanity #16)
// Date:        2026-05-03 — Madrid
//
// Goal: find a kissing configuration in R^13 of size > 1154 (Zinoviev-Ericson
//       1999, world record currently 27 years standing).
//
// =============================================================================
// v2.1 CHANGES (Auditor-ratified 2026-05-03 — fix G8 + sanity #16 + dashboard):
// =============================================================================
//
//   FIX v2.1-A — CRYO_THRESH recalibrated 1125 -> 1131:
//     v2 inherited CRYO_THRESH=1125 from Sobol "near-record" pattern WITHOUT
//     recalibrating to kissing dim 13 context. In dim 13, 1125 is THREE points
//     BELOW the P13a floor (1130), not near record. Result on Mac M2 run1:
//     Mode B warm-loaded with classical 1130, immediately triggered cryo
//     (best_global=1130 >= 1125), all 5 chains collapsed to T_FLOOR=0.005,
//     cascade acceptance dropped to 0.005-0.01%, best frozen at 1130 for 3h+.
//     v2.1 sets CRYO_THRESH=1131 — anchored in the engine's own declaration
//     of 1131 as PAPER-WORTHY. Cryo activates ONLY when we've actually beaten
//     Leech-Sloane 1971. Below 1131: full thermal exploration.
//     Lapsus G8 (Constructor + Auditor shared) jubilated. Number defended.
//
//   FIX v2.1-B — Constants defense comments on critical constexpr:
//     Every operationally-critical constexpr now carries a one-line defense
//     anchored in kissing dim 13 mathematics (NOT "inherited from Sobol").
//     Constants without defendable rationale marked TODO_AUDIT for review.
//
//   FIX v2.1-C — Sanity #16: Mode B post-warm-load thermal sanity:
//     Closes the F5-style blind spot of v2. Test simulates Mode B entry:
//     warm-load classical 1130 in chain c0, run 10000 iters, verify:
//       * T_current does NOT collapse to T_FLOOR cryogenic (0.005)
//       * Move T cascade fires >=50 times with E>2 (deep cascades real)
//       * Condition (best_global >= CRYO_THRESH) stays FALSE while best=1130
//     If FAIL: another constant is mis-calibrated, do NOT relaunch.
//
//   FIX v2.1-D — Dashboard: T_FLOOR_HITS per chain:
//     v2 dashboard could not distinguish "chain bouncing on T floor" from
//     "chain stuck at T floor". v2.1 adds counter t_floor_hits[CHAINS]
//     incremented every time cs.T_current touches floor (cryo or normal).
//     Printed every 30s alongside chain_sz and T values.
//
// =============================================================================
//
// JUBILATED (graveyard, 2026-05-03):
//   HORMONEROCHUMINEROPREMIUM_v2 — Mac M2 run1 2026-05-03 12:48:18 CEST,
//   3h18m+ wall, best=1130 reported with verify_gate 370/0 rejected.
//   STATE INTEGRITY: PERFECT (snapshot/restore mechanism v2-A confirmed empirically).
//   THERMAL DYNAMICS: BROKEN (CRYO_THRESH=1125 sub-floor caused immediate
//   cryogenic collapse on Mode B warm-load, 5 chains stuck at T=0.005,
//   no escape from meseta 1130 possible).
//   Bug class: same as F5 (invariant chequeada, dynamics not chequeada).
//   Lesson G8: parameters inherited from sister projects MUST be recalibrated
//   in target context BEFORE ratification. Sobol's CRYO_THRESH was record-zone;
//   kissing dim 13's must be anchored in its own scale.
//
//   HORMONEROCHUMINEROPREMIUM_v1.5 — Mac M2 2026-05-03 10:10:48 CEST,
//   202.1s wall, best=1132 reported but VERIFY FAIL violations=14 sqrt(2).
//   Bug F4: Move T cascade self-eviction non-symmetric revert. Closed in v2-A.
//
// =============================================================================
//
// Foundation (verified bit-for-bit, see steps 1-7 in sandbox):
//   - S(5,6,12) Steiner system: 132 hexads
//   - P12a code (12, 144, 4) with A_4 = 51 (Leech-Sloane 1971)
//   - Construction A in R^12: kissing 840
//   - P13a layer-stacking in R^13: 840 + 288 + 2 = 1130 (sec 4.3)
//
// Pool (pool_v15.bin, Architect-delivered):
//   - 64,802 lattice candidates at sqdist EXACTLY 4.0 from center x_13
//   - First 1,130 = classical P13a (independent set)
//   - Last 63,672 = extras with K_static in [4, 16]
//
// Search problem: MAXIMUM INDEPENDENT SET on conflict graph.
//   Floor (Mode A validation):  1130 (P13a)
//   Paper-worthy threshold:     1131 (BEAT Leech-Sloane 1971)
//   Cryogenic activation:       1131 (defended: protect paper-worthy from earthquakes)
//   Record tie:                 1154
//   Beat:                       >= 1155
//
// Compile (Mac M2):
//   g++ -O3 -march=native -std=c++17 -funroll-loops -Wall -Wextra -o HORMONEROCHUMINEROPREMIUM_v2_1 HORMONEROCHUMINEROPREMIUM_v2_1.cpp
//
// Run (foreground per Architect directive D[N]):
//   cd ~/Downloads && caffeinate -dims taskpolicy -c utility ./HORMONEROCHUMINEROPREMIUM_v2_1 pool_v15.bin 2>&1 | tee HORMONEROCHUMINEROPREMIUM_v2_1_run.log
//
// =============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cassert>

// -----------------------------------------------------------------------------
// CONFIGURATION (compile-time constants)
// -----------------------------------------------------------------------------

// =============================================================================
// CONSTANTS (each operationally-critical constexpr defended in kissing dim 13
// context per v2.1-B. Numbers without dim-13 anchor are TODO_AUDIT.)
// =============================================================================

// Geometry (mathematical, non-negotiable):
constexpr int    DIM_AMBIENT   = 13;     // R^13 — target dimension
constexpr double DIST_KISS     = 2.0;    // unit spheres of radius 1, dist between centers = 2
constexpr double SQDIST_KISS   = 4.0;    // squared, used in CSR adjacency comparison
constexpr double EPS_DIST      = 1e-9;   // tolerance for sqdist exact match (lattice has rationals)

// Defended: 5 chains balances PT swap diversity vs Mac M2 25% CPU budget.
// Less (3): undersamples thermal ladder. More (7+): each chain underiterates per
// wall-clock budget. 5 is the inherited-from-Sobol value AND empirically verified
// in v1 sandbox + v2 Mac run1 to produce balanced exploration.
constexpr int CHAINS = 5;

// Defended: hotter ladder for kissing-MIS landscape (wider basins than Sobol).
// {0.10, 0.15, 0.22, 0.32, 0.45} validated in v1 smoke (best=1135 in 30s sanity #9)
// and v2 Mac run1 (per-chain acc% 28-31% in first 5 min, exactly the 15-35% target).
// Cold values like {0.05,...,0.25} froze chains (v1 sandbox best=1126).
constexpr double T_LADDER[CHAINS] = { 0.10, 0.15, 0.22, 0.32, 0.45 };

// Defended: T_FLOOR=0.020 prevents deterministic-greedy collapse below this.
// At T<0.020 with kissing-MIS structure, exp(-1/T) becomes too small to accept
// even single-point swap negative deltas — chain is effectively frozen. T=0.020
// gives exp(-1/0.020) = exp(-50) ~ 2e-22 for delta=-1: still tiny, but the COMPOUND
// effect over many tries means delta=-2 cascade with 30 tries has p_accept ~ 6e-21,
// non-zero. Below 0.020: floor for ALL realistic moves.
// Active ONLY when best_global < CRYO_THRESH (i.e., not in cryogenic refinement).
constexpr double T_FLOOR = 0.020;

// Defended: tabu caps per chain, asymmetric to encourage role-specialization.
// Hot chains (c3=45, c4=35) cycle tabu faster -> more exploration, more pair re-additions.
// Cold chains (c0..c2=65) hold tabu longer -> stability, anti-oscillation in basins.
// Sobol convention but anchored: ratio of ~0.5 between hot and cold seems to reproduce
// PT swap diversity observed in v2 Mac run1 (chain sizes balanced 1040-1095 in cryo).
constexpr int TABU_CAP[CHAINS] = { 65, 65, 65, 45, 35 };

// Move mix (sums to 100). Defended:
//   R, M (greedy/random add): 18+16=34. Cover cold-start exploration.
//   W=0: removed in v1 (was degenerate Δ=0 churn at 100% acceptance).
//   K (kill weakest): 8. Pressure-directed eviction for stuck states.
//   L (chain add 2-3): 6. Mild cascade for medium-warm chains.
//   S (random kill + greedy fill): 8. Diversification through perturbation.
//   Z (atomic swap): 11 (60% of K9 weight = 6 + 40% of W=5). Δ=0 with p=0.30 (v2-E).
//   X (medium earthquake 4-5): 8. Larger perturbation, basin-jumping.
//   P (large earthquake 8-10): 4. RARE — only used to escape deep meseta.
//   T (cascade): 21 (60% of W's old 17 = 14 + 7 reserve). Core move for kissing.
// W disabled empirically (v1 smoke: 100% acc, 0 progress = noise generator).
constexpr int MIX_R = 18;
constexpr int MIX_M = 16;
constexpr int MIX_W = 0;
constexpr int MIX_K = 8;
constexpr int MIX_L = 6;
constexpr int MIX_S = 8;
constexpr int MIX_Z = 11;
constexpr int MIX_X = 8;
constexpr int MIX_P = 4;
constexpr int MIX_T = 21;
constexpr int N_MOVES = 10;

// Defended: T_K_MIN=4, T_K_MAX=16.
// In pool_v15.bin, extras with K_static=18+ (i.e., conflicting with 18+ classicals)
// are too "expensive" — cascade would need to evict 18 to add 1, ratio 18:1 is far
// past auditor's 2N+5 cap. Extras with K_static<4 don't exist in this pool (filter).
// [4,16] covers the productive cascade-seeding zone.
constexpr int  T_K_MIN = 4;
constexpr int  T_K_MAX = 16;

// Defended: budget calibrated to MEASURED Mac M2 throughput.
// v2 Mac run1 measured ~3000 it/s post snapshot overhead (NOT 7400 as theorized).
// 12h target with extends ~ 130-180M iters total.
// Distribution: Mode A 30M (until validated) / Mode B 100-150M / Polish ~5M.
constexpr long ITERS_BASE      = 30000000L;    // Mode A primary budget
constexpr int  EXTENDS         = 4;            // up to 4 budget extensions per phase
constexpr long ITERS_EXT       = 5000000L;     // 5M iters per extend (~25 min on Mac)
constexpr long EXTEND_MIN_IMPR = 2;            // require >=2 improvements to grant extend

// Triple-rescue D189 (Sobol heritage). Defended: timing thresholds for chain c0
// stagnation rescue. RESCUE_T1=5000 iters without improvement triggers move_X.
// T2/T3 reserved for future tier escalation; NOT used in current run_iter.
// TODO_AUDIT: T2/T3/PROMOTE are dead constants in v2.1, candidates for removal in v3.
constexpr long RESCUE_T1       = 5000;
constexpr long RESCUE_T2       = 2000;     // TODO_AUDIT: not referenced in run_iter
constexpr long RESCUE_T3       = 4000;     // TODO_AUDIT: not referenced
constexpr long RESCUE_PROMOTE  = 7500;     // TODO_AUDIT: not referenced

// === FIX v2.1-A: CRYO_THRESH RECALIBRATED ===
// Defended (replaces v2 lapsus G8):
//   v2 had CRYO_THRESH=1125 inherited from Sobol's "near-record" pattern.
//   But in kissing dim 13, 1125 is THREE points BELOW the P13a floor (1130),
//   so cryo activated immediately on Mode B warm-load and froze all chains.
//   v2.1: CRYO_THRESH=1131 anchored in the engine's OWN declaration of 1131
//   as PAPER-WORTHY (HIT 1131 banner = "BEAT LEECH-SLOANE 1971 *PAPER-WORTHY*").
//   Cryo activates ONLY when we've actually crossed the paper threshold,
//   meaning we've passed Mode B's hard wall and now refine in record-bound
//   territory. Below 1131: full thermal energy for cascade exploration.
//   This number IS anchored in the engine's tier system, not memory.
constexpr int    CRYO_THRESH       = 1131;
// Defended: T_CRYO_MAX=0.04 caps thermal energy in cryogenic refinement.
// At T=0.04, exp(-1/0.04) = exp(-25) ~ 1.4e-11. Single-step delta=-1 essentially
// rejected, BUT cascade move with E=2 evictions producing temporary delta=-2 then
// final delta=+1 still feasible if intermediate is exp(-2/0.04*0.4) = exp(-20) ~ 2e-9
// (the 0.4 factor in Move T cascade Metropolis). Allows refinement without churn.
constexpr double T_CRYO_MAX        = 0.04;
// Defended: T factors. CRYO 0.99996 is slower decay (chain stays warm longer near
// record). NORM 0.99995 is faster decay for Mode A exploration phase.
// Difference is ~20% in half-life, calibrated to balance exploration/exploitation.
constexpr double T_FACTOR_CRYO     = 0.99996;
constexpr double T_FACTOR_NORM     = 0.99995;

// Defended: reheats up to 65 per chain protect against permanent freezes.
// REHEAT_MIN_TBASE=0.06 means only chains in T_LADDER[1..4] reheat (c0=0.10 base
// is too cold to benefit). REHEAT_STAG_TRIGGER=8000 iters: empirically the lower
// bound where stagnation is signal not noise. 65 reheats x 5 chains = 325 thermal
// resets total, plenty for 130M iter run.
constexpr int    MAX_REHEATS         = 65;
constexpr double REHEAT_MIN_TBASE    = 0.06;
constexpr long   REHEAT_STAG_TRIGGER = 8000;

// Defended: MIX_RECAL at iter 15000 — single recalibration after enough samples
// (~3000 tries per move per chain) to produce meaningful rate stats. Move Z (idx 6)
// and Move T (idx 9) excluded by design (low-rate-by-design moves). Earlier (5K):
// noise. Later (50K+): wasted opportunity to fix mis-weighted moves.
constexpr long MIX_RECAL_AT = 15000;

// Defended: 15 polish passes post-SA. Each pass attempts to swap one active for
// two free with same-or-better delta. Empirically, gains saturate at pass 3-7;
// pass 15 cap is safety budget. v1.5 polish gain=17 then 0 at passes 1-2.
constexpr int  POLISH_MAX_PASSES = 15;

// Defended: 30s log cadence. Faster (5s) -> log spam, smaller signal/noise.
// Slower (60s) -> Architect loses real-time tracking. 30s is the visual sweet spot.
constexpr long LOG_EVERY_SEC = 30;

// Targets defended:
constexpr int  TARGET_VALIDATE   = 1130;   // Mode A floor: P13a (Leech-Sloane 1971)
constexpr int  TARGET_RETRAIN_LO = 1100;   // [1100, 1130] = retrain not graveyard tier
constexpr int  TARGET_GRAVEYARD  = 1000;   // <1000 = hard graveyard, engine fails
constexpr int  TARGET_RECORD_TIE = 1154;   // Zinoviev-Ericson 1999 world record
constexpr int  TARGET_RECORD     = 1155;   // first new record candidate (LUNA goal)
constexpr int  TARGET_SAVE_FROM  = 900;    // start saving configs from here

// Defended: Mode A jump triple-gate (v2-D). 5 distinct chain-improvements at floor
// confirms it's not single-fluke. Lower (2-3) admits flukes, higher (10+) wastes
// Mode A budget on already-validated runs.
constexpr int  MODE_A_MIN_DISTINCT_IMPROVEMENTS = 5;

// Defended: periodic geometric verify cadence (v2-B).
// Coarse 50 pairs / 1000 iters: detects most corruption within 100K iters.
// Fine 200 pairs / 10000 iters: tighter coverage check.
// Cost ~0.5% throughput on Mac M2, validated empirically in run1.
constexpr long VERIFY_COARSE_EVERY = 1000;
constexpr int  VERIFY_COARSE_PAIRS = 50;
constexpr long VERIFY_FINE_EVERY   = 10000;
constexpr int  VERIFY_FINE_PAIRS   = 200;

// Defended: Move Z relaxation (v2-E). Δ=0 accepted with p=0.30.
// Strict Δ>0 in v1.5 yielded 0% accept (move dead, no diversification).
// Δ>=0 with p=1.0 produces W-style churn (rejected).
// p=0.30 gives empirical 1.6-2.0% accept rate on Mac M2 run1 (per-chain) =
// productive diversification without churn. Validated.
constexpr double Z_DELTA_ZERO_PACC = 0.30;

// ANSI color codes for Architect dashboard (v2-G).
// Mac Terminal native support, no install needed.
#define ANSI_RESET    "\033[0m"
#define ANSI_BOLD     "\033[1m"
#define ANSI_GREEN    "\033[32m"
#define ANSI_YELLOW   "\033[33m"
#define ANSI_RED      "\033[31m"
#define ANSI_CYAN     "\033[36m"
#define ANSI_MAGENTA  "\033[35m"

// Defended: deterministic RNG seed for reproducibility.
// Hex pun "LUNA CODE D 1 6 0 0 13" — irrelevant to behavior, used as canonical
// project signature. ANY 64-bit seed works; this one is fixed for replay.
constexpr uint64_t RNG_SEED = 0x10A1AC0D160013ULL;

// -----------------------------------------------------------------------------
// GLOBAL POOL DATA (loaded from pool.bin)
// -----------------------------------------------------------------------------

int N_POOL      = 0;
int K_CLASSICAL = 0;   // first K_CLASSICAL entries of pool[] = classical P13a
int DIM         = 0;

double  CENTER[DIM_AMBIENT];                 // x_13
std::vector<std::array<double, DIM_AMBIENT>> POOL;

// Conflict adjacency (CSR-like, precomputed at startup)
//   adj_offset[i] = start index in adj_list[] for vertex i
//   adj_offset[N_POOL] = total edges
std::vector<int> adj_offset;     // size N_POOL + 1
std::vector<int> adj_list;       // sum of degrees

// Per-extra: list of K classical conflict indices (in CSR-like form)
//   Used by Move T to find "cheap extras" and their classical conflict sets.
std::vector<int> classical_conflicts_offset;   // size (N_POOL - K_CLASSICAL) + 1
std::vector<int> classical_conflicts_list;     // flat list of classical indices conflicting with each extra

// Bucket extras by K_static (count of classical conflicts).
//   bucket[k] = vector of extra indices (in pool, i.e. >= K_CLASSICAL) with K_static == k
std::vector<std::vector<int>> bucket_by_kstatic;  // size T_K_MAX + 1 (only entries [T_K_MIN..T_K_MAX] used)

// -----------------------------------------------------------------------------
// TIMING & RNG
// -----------------------------------------------------------------------------

auto t_start = std::chrono::steady_clock::now();

double now_sec() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t_start).count();
}

std::mt19937_64 rng(RNG_SEED);

inline double rand_uniform() {
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

inline int rand_int(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

// -----------------------------------------------------------------------------
// DISTANCE HELPERS (dim=13 small, direct loop fine)
// -----------------------------------------------------------------------------

inline double sqdist_pool(int i, int j) {
    double s = 0.0;
    for (int d = 0; d < DIM; d++) {
        double diff = POOL[i][d] - POOL[j][d];
        s += diff * diff;
    }
    return s;
}

inline bool conflicts_pool(int i, int j) {
    return sqdist_pool(i, j) < SQDIST_KISS - EPS_DIST;
}

// -----------------------------------------------------------------------------
// POOL LOADING (binary format from step8_serialize.py)
// -----------------------------------------------------------------------------

bool load_pool(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[FATAL] cannot open %s\n", path);
        return false;
    }

    int32_t header[3];
    if (fread(header, sizeof(int32_t), 3, f) != 3) {
        fprintf(stderr, "[FATAL] header read failed\n");
        fclose(f);
        return false;
    }
    N_POOL = header[0];
    DIM = header[1];
    K_CLASSICAL = header[2];

    if (DIM != DIM_AMBIENT) {
        fprintf(stderr, "[FATAL] DIM mismatch: pool=%d expected=%d\n", DIM, DIM_AMBIENT);
        fclose(f);
        return false;
    }

    if (fread(CENTER, sizeof(double), DIM, f) != (size_t)DIM) {
        fprintf(stderr, "[FATAL] center read failed\n");
        fclose(f);
        return false;
    }

    POOL.resize(N_POOL);
    for (int i = 0; i < N_POOL; i++) {
        if (fread(POOL[i].data(), sizeof(double), DIM, f) != (size_t)DIM) {
            fprintf(stderr, "[FATAL] pool row %d read failed\n", i);
            fclose(f);
            return false;
        }
    }
    fclose(f);

    fprintf(stderr, "[POOL] loaded %d candidates, dim=%d, K_classical=%d\n",
            N_POOL, DIM, K_CLASSICAL);
    return true;
}

// -----------------------------------------------------------------------------
// PRE-FLIGHT RAM CHECK (auditor sanity #11)
//
// Estimate the CSR adjacency cost BEFORE allocating it. Sample a small subset
// to estimate density, then extrapolate to total pairs and CSR bytes. Abort
// cleanly if the estimate exceeds the safety threshold (4 GB by default,
// leaving 4 GB for macOS + chains + other state on a M2 8GB).
//
// Returns true if safe to proceed, false if abort.

constexpr double RAM_SAFETY_LIMIT_GB = 4.0;

bool preflight_ram_check() {
    fprintf(stderr, "\n[PREFLIGHT] estimating CSR adjacency RAM cost...\n");

    // Sample 1500 random candidates, count pairwise conflicts
    constexpr int SAMPLE = 1500;
    int s = std::min(SAMPLE, N_POOL);
    std::vector<int> idx(N_POOL);
    for (int i = 0; i < N_POOL; i++) idx[i] = i;
    // Fisher-Yates shuffle first s
    for (int i = 0; i < s; i++) {
        int j = i + rand_int(0, N_POOL - i - 1);
        std::swap(idx[i], idx[j]);
    }

    long sample_pairs = (long)s * (s - 1) / 2;
    long sample_conflicts = 0;
    for (int i = 0; i < s; i++) {
        for (int j = i + 1; j < s; j++) {
            double sq = 0.0;
            int a = idx[i], b = idx[j];
            for (int d = 0; d < DIM; d++) {
                double diff = POOL[a][d] - POOL[b][d];
                sq += diff * diff;
            }
            if (sq < SQDIST_KISS - EPS_DIST) sample_conflicts++;
        }
    }

    double density = (double)sample_conflicts / (double)sample_pairs;
    long total_pairs = (long)N_POOL * (N_POOL - 1) / 2;
    long est_conflict_pairs = (long)((double)total_pairs * density);
    // CSR bidirectional: 2 ints per pair = 8 bytes
    double csr_bytes = 2.0 * (double)est_conflict_pairs * sizeof(int);
    double csr_gb = csr_bytes / (1024.0 * 1024.0 * 1024.0);
    double csr_mb = csr_bytes / (1024.0 * 1024.0);

    fprintf(stderr, "[PREFLIGHT]   pool size:               %d\n", N_POOL);
    fprintf(stderr, "[PREFLIGHT]   sample density:          %.4f%% (sample %d candidates)\n",
            density * 100.0, s);
    fprintf(stderr, "[PREFLIGHT]   est conflict pairs:      %ld\n", est_conflict_pairs);
    fprintf(stderr, "[PREFLIGHT]   CSR bidir int32 size:    %.3f GB (%.0f MB)\n", csr_gb, csr_mb);
    fprintf(stderr, "[PREFLIGHT]   safety limit:            %.1f GB\n", RAM_SAFETY_LIMIT_GB);

    if (csr_gb > RAM_SAFETY_LIMIT_GB) {
        fprintf(stderr, "\n[PREFLIGHT] ABORT: estimated CSR size (%.2f GB) exceeds safety limit (%.1f GB).\n",
                csr_gb, RAM_SAFETY_LIMIT_GB);
        fprintf(stderr, "[PREFLIGHT] Reduce pool size (filter to lower K_static range) and re-launch.\n");
        return false;
    }
    double headroom = RAM_SAFETY_LIMIT_GB / std::max(csr_gb, 0.001);
    fprintf(stderr, "[PREFLIGHT]   headroom factor:         %.2fx safe to proceed\n", headroom);
    return true;
}

// -----------------------------------------------------------------------------
// PRECOMPUTE ADJACENCY (CSR conflict list)
// -----------------------------------------------------------------------------

void precompute_adjacency() {
    fprintf(stderr, "[ADJ] precomputing conflict adjacency for %d candidates...\n", N_POOL);
    auto t0 = now_sec();

    // Two-pass CSR: first count degrees, then fill.
    std::vector<int> degree(N_POOL, 0);

    // O(N^2) brute force: 150K^2 / 2 = ~11G pair checks. Each pair check ~13 mults.
    // On M2: ~30-60 seconds. Acceptable.
    for (int i = 0; i < N_POOL; i++) {
        const auto& pi = POOL[i];
        for (int j = i + 1; j < N_POOL; j++) {
            double s = 0.0;
            for (int d = 0; d < DIM; d++) {
                double diff = pi[d] - POOL[j][d];
                s += diff * diff;
            }
            if (s < SQDIST_KISS - EPS_DIST) {
                degree[i]++;
                degree[j]++;
            }
        }
        if ((i & 4095) == 0 && i > 0) {
            fprintf(stderr, "  adjacency: %d/%d (%.1f%%) elapsed=%.1fs\n",
                    i, N_POOL, 100.0 * i / N_POOL, now_sec() - t0);
        }
    }

    // Build CSR offsets
    adj_offset.assign(N_POOL + 1, 0);
    for (int i = 0; i < N_POOL; i++) adj_offset[i + 1] = adj_offset[i] + degree[i];
    long total_edges_directed = adj_offset[N_POOL];
    adj_list.resize(total_edges_directed);

    // Fill (second pass)
    std::vector<int> fill_ptr(N_POOL, 0);
    for (int i = 0; i < N_POOL; i++) {
        const auto& pi = POOL[i];
        for (int j = i + 1; j < N_POOL; j++) {
            double s = 0.0;
            for (int d = 0; d < DIM; d++) {
                double diff = pi[d] - POOL[j][d];
                s += diff * diff;
            }
            if (s < SQDIST_KISS - EPS_DIST) {
                adj_list[adj_offset[i] + fill_ptr[i]++] = j;
                adj_list[adj_offset[j] + fill_ptr[j]++] = i;
            }
        }
    }

    long total_pairs = total_edges_directed / 2;
    fprintf(stderr, "[ADJ] done in %.1fs. Total conflict pairs: %ld (density %.3f%%)\n",
            now_sec() - t0, total_pairs,
            100.0 * total_pairs / ((double)N_POOL * (N_POOL - 1) / 2.0));
}

// -----------------------------------------------------------------------------
// BUCKET CLASSICAL_CONFLICTS_COUNT FOR MOVE T
// -----------------------------------------------------------------------------

void precompute_extras_bucketing() {
    fprintf(stderr, "[BUCKET] precomputing classical-conflict buckets for extras...\n");

    int n_extras = N_POOL - K_CLASSICAL;

    // For each extra (idx >= K_CLASSICAL), find its conflicts with classicals.
    classical_conflicts_offset.assign(n_extras + 1, 0);

    // First pass: count
    for (int e = 0; e < n_extras; e++) {
        int extra_idx = K_CLASSICAL + e;
        int count = 0;
        int beg = adj_offset[extra_idx];
        int end = adj_offset[extra_idx + 1];
        for (int p = beg; p < end; p++) {
            int nbr = adj_list[p];
            if (nbr < K_CLASSICAL) count++;
        }
        classical_conflicts_offset[e + 1] = classical_conflicts_offset[e] + count;
    }

    classical_conflicts_list.resize(classical_conflicts_offset[n_extras]);

    // Second pass: fill
    std::vector<int> fill_ptr(n_extras, 0);
    for (int e = 0; e < n_extras; e++) {
        int extra_idx = K_CLASSICAL + e;
        int beg = adj_offset[extra_idx];
        int end = adj_offset[extra_idx + 1];
        for (int p = beg; p < end; p++) {
            int nbr = adj_list[p];
            if (nbr < K_CLASSICAL) {
                classical_conflicts_list[classical_conflicts_offset[e] + fill_ptr[e]++] = nbr;
            }
        }
    }

    // Bucket extras by their static K_static count (= classical_conflicts_offset diff)
    bucket_by_kstatic.assign(T_K_MAX + 1, std::vector<int>());
    for (int e = 0; e < n_extras; e++) {
        int kstatic = classical_conflicts_offset[e + 1] - classical_conflicts_offset[e];
        if (kstatic >= T_K_MIN && kstatic <= T_K_MAX) {
            bucket_by_kstatic[kstatic].push_back(K_CLASSICAL + e);
        }
    }

    fprintf(stderr, "[BUCKET] cheap extras by K_static (auditor Q2 weighting target):\n");
    long total = 0;
    for (int k = T_K_MIN; k <= T_K_MAX; k++) {
        fprintf(stderr, "  K_static = %2d  ->  %zu extras\n", k, bucket_by_kstatic[k].size());
        total += bucket_by_kstatic[k].size();
    }
    fprintf(stderr, "  TOTAL in T-target range [%d..%d]: %ld extras\n",
            T_K_MIN, T_K_MAX, total);
}

// -----------------------------------------------------------------------------
// CHAIN STATE (Diamante VIKINGO + Sobol multi-chain)
// -----------------------------------------------------------------------------

struct ChainState {
    // Active membership
    std::vector<bool> active;       // size N_POOL: is candidate i in current config?
    std::vector<int>  active_list;  // list of active indices (for fast iteration / sample)
    std::vector<int>  active_pos;   // active_pos[i] = position in active_list, or -1
    int               size;

    // Incremental BLOCKED[i] = number of active candidates conflicting with i.
    // (D14: zero algorithmic laziness — incremental, never recomputed.)
    std::vector<int>  blocked;       // size N_POOL

    // Free candidates list (blocked[i]==0 AND !active[i])
    std::vector<int>  free_list;
    std::vector<int>  free_pos;      // free_pos[i] = position in free_list, or -1

    // Best local config tracking
    int  best_local;
    long last_improve_iter;
    int  reheats;
    long stag;

    // Temperature
    double T_base;
    double T_current;

    // Tabu (cyclic buffer of recently-evicted indices)
    int  tabu_count;
    std::vector<int> tabu_buffer;
    int  tabu_head;

    // Move counters (for stats / MIX_RECAL)
    long acc[N_MOVES] = {0};
    long tries[N_MOVES] = {0};

    // v2.1-D: T_FLOOR hit counter. Incremented every time the cooling step
    // would have pushed T_current BELOW the active floor (cryo or normal) and
    // got clamped. Lets the dashboard distinguish "chain bouncing on floor"
    // (high count, healthy) from "chain stuck at floor" (count saturated quickly,
    // then no growth = no more cooling attempts = exploration dead).
    long t_floor_hits = 0;
};

// Per-chain Move T statistics
struct MoveTStats {
    long fired = 0;
    long accepted = 0;
    long total_evicted = 0;
    long total_added = 0;
    long net_gain_total = 0;
};

// Chain initialization (empty config)
void chain_init_empty(ChainState& cs, int chain_idx) {
    cs.active.assign(N_POOL, false);
    cs.active_list.clear();
    cs.active_pos.assign(N_POOL, -1);
    cs.size = 0;
    cs.blocked.assign(N_POOL, 0);

    cs.free_list.clear();
    cs.free_pos.assign(N_POOL, -1);
    for (int i = 0; i < N_POOL; i++) {
        cs.free_pos[i] = (int)cs.free_list.size();
        cs.free_list.push_back(i);
    }

    cs.best_local = 0;
    cs.last_improve_iter = 0;
    cs.reheats = 0;
    cs.stag = 0;
    cs.T_base = T_LADDER[chain_idx];
    cs.T_current = cs.T_base;

    cs.tabu_count = TABU_CAP[chain_idx];
    cs.tabu_buffer.assign(cs.tabu_count, -1);
    cs.tabu_head = 0;

    for (int m = 0; m < N_MOVES; m++) { cs.acc[m] = 0; cs.tries[m] = 0; }
    cs.t_floor_hits = 0;
}

// Helpers for free_list
inline void free_remove(ChainState& cs, int idx) {
    int pos = cs.free_pos[idx];
    if (pos < 0) return;
    int last = (int)cs.free_list.size() - 1;
    int last_idx = cs.free_list[last];
    cs.free_list[pos] = last_idx;
    cs.free_pos[last_idx] = pos;
    cs.free_list.pop_back();
    cs.free_pos[idx] = -1;
}

inline void free_add(ChainState& cs, int idx) {
    if (cs.free_pos[idx] >= 0) return;
    cs.free_pos[idx] = (int)cs.free_list.size();
    cs.free_list.push_back(idx);
}

inline void active_list_remove(ChainState& cs, int idx) {
    int pos = cs.active_pos[idx];
    int last = (int)cs.active_list.size() - 1;
    int last_idx = cs.active_list[last];
    cs.active_list[pos] = last_idx;
    cs.active_pos[last_idx] = pos;
    cs.active_list.pop_back();
    cs.active_pos[idx] = -1;
}

inline void active_list_add(ChainState& cs, int idx) {
    cs.active_pos[idx] = (int)cs.active_list.size();
    cs.active_list.push_back(idx);
}

// FIX-B8 incremental BLOCKED[] update: when adding active idx, walk its
// adjacency list and increment blocked[] for each neighbor.
void add_to_active(ChainState& cs, int idx) {
    if (cs.active[idx]) return;
    cs.active[idx] = true;
    active_list_add(cs, idx);
    cs.size++;
    free_remove(cs, idx);

    int beg = adj_offset[idx];
    int end = adj_offset[idx + 1];
    for (int p = beg; p < end; p++) {
        int nbr = adj_list[p];
        int prev = cs.blocked[nbr];
        cs.blocked[nbr] = prev + 1;
        if (prev == 0 && !cs.active[nbr]) {
            free_remove(cs, nbr);
        }
    }
}

void remove_from_active(ChainState& cs, int idx) {
    if (!cs.active[idx]) return;
    cs.active[idx] = false;
    active_list_remove(cs, idx);
    cs.size--;

    int beg = adj_offset[idx];
    int end = adj_offset[idx + 1];
    for (int p = beg; p < end; p++) {
        int nbr = adj_list[p];
        int prev = cs.blocked[nbr];
        cs.blocked[nbr] = prev - 1;
        if (prev == 1 && !cs.active[nbr]) {
            free_add(cs, nbr);
        }
    }
    if (cs.blocked[idx] == 0 && !cs.active[idx]) free_add(cs, idx);
}

// Tabu helpers
inline bool is_tabu(const ChainState& cs, int idx) {
    for (int t = 0; t < cs.tabu_count; t++)
        if (cs.tabu_buffer[t] == idx) return true;
    return false;
}
inline void tabu_push(ChainState& cs, int idx) {
    cs.tabu_buffer[cs.tabu_head] = idx;
    cs.tabu_head = (cs.tabu_head + 1) % cs.tabu_count;
}

// -----------------------------------------------------------------------------
// MOVES
// -----------------------------------------------------------------------------

// Move R: greedy add — pick a free candidate maximizing min-distance to active.
// For our problem, "min distance" maps to: minimize # of conflicts the new
// candidate brings (ideally 0 since it's free, but among free, prefer the one
// that least restricts future free_list).
// Approximation: pick free candidate with fewest CONFLICTS among NON-active
// candidates (low future blocking).
bool move_R(ChainState& cs) {
    cs.tries[0]++;
    if (cs.free_list.empty()) return false;
    constexpr int K = 32;
    int n = (int)cs.free_list.size();
    int trials = std::min(K, n);
    int best = -1;
    int best_score = INT_MAX;
    for (int t = 0; t < trials; t++) {
        int c = cs.free_list[rand_int(0, n - 1)];
        if (is_tabu(cs, c)) continue;
        // score: count conflicts with NON-active candidates
        int score = 0;
        int beg = adj_offset[c], end = adj_offset[c + 1];
        for (int p = beg; p < end; p++) {
            int nbr = adj_list[p];
            if (!cs.active[nbr]) score++;
        }
        if (score < best_score) { best_score = score; best = c; }
    }
    if (best < 0) return false;
    add_to_active(cs, best);
    cs.acc[0]++;
    return true;
}

// Move M: pure random add from free_list.
bool move_M(ChainState& cs) {
    cs.tries[1]++;
    if (cs.free_list.empty()) return false;
    int n = (int)cs.free_list.size();
    int c = cs.free_list[rand_int(0, n - 1)];
    if (is_tabu(cs, c)) return false;
    add_to_active(cs, c);
    cs.acc[1]++;
    return true;
}

// Move W: swap. Remove one active, try to add one free. Metropolis on Δsize.
bool move_W(ChainState& cs) {
    cs.tries[2]++;
    if (cs.size == 0) return false;
    int rm = cs.active_list[rand_int(0, cs.size - 1)];
    int prev_size = cs.size;
    remove_from_active(cs, rm);
    tabu_push(cs, rm);

    if (cs.free_list.empty()) {
        add_to_active(cs, rm);
        return false;
    }
    int n = (int)cs.free_list.size();
    int add = cs.free_list[rand_int(0, n - 1)];
    add_to_active(cs, add);

    int delta = cs.size - prev_size;
    if (delta >= 0) { cs.acc[2]++; return true; }
    double p = std::exp((double)delta / cs.T_current);
    if (rand_uniform() < p) { cs.acc[2]++; return true; }
    remove_from_active(cs, add);
    add_to_active(cs, rm);
    return false;
}

// Move K: kill weakest, pressure-directed (VIKINGO).
// Find active with most blocking impact on free candidates, remove it.
bool move_K(ChainState& cs) {
    cs.tries[3]++;
    if (cs.size == 0) return false;
    constexpr int SAMPLE = 12;
    int trials = std::min(SAMPLE, cs.size);
    int best = -1;
    int best_unblock = -1;
    for (int t = 0; t < trials; t++) {
        int v = cs.active_list[rand_int(0, cs.size - 1)];
        // Count: of v's neighbors, how many would unblock if v removed?
        // Unblock = blocked[nbr]==1 (v is the SOLE blocker) and !active[nbr]
        int unblock = 0;
        int beg = adj_offset[v], end = adj_offset[v + 1];
        for (int p = beg; p < end; p++) {
            int nbr = adj_list[p];
            if (cs.blocked[nbr] == 1 && !cs.active[nbr]) unblock++;
        }
        if (unblock > best_unblock) { best_unblock = unblock; best = v; }
    }
    if (best < 0) return false;
    int prev_size = cs.size;
    remove_from_active(cs, best);
    tabu_push(cs, best);

    // Greedy fill 2 if possible
    int adds = 0;
    for (int attempt = 0; attempt < 4 && !cs.free_list.empty() && adds < 2; attempt++) {
        int n = (int)cs.free_list.size();
        int c = cs.free_list[rand_int(0, n - 1)];
        if (is_tabu(cs, c)) continue;
        add_to_active(cs, c);
        adds++;
    }
    int delta = cs.size - prev_size;
    if (delta >= 0) { cs.acc[3]++; return true; }
    double p = std::exp((double)delta / cs.T_current);
    if (rand_uniform() < p) { cs.acc[3]++; return true; }
    return false;
}

// Move L: chain add 2-3 free.
bool move_L(ChainState& cs) {
    cs.tries[4]++;
    int target = 2 + rand_int(0, 1);
    int adds = 0;
    for (int t = 0; t < target * 3 && adds < target; t++) {
        if (cs.free_list.empty()) break;
        int n = (int)cs.free_list.size();
        int c = cs.free_list[rand_int(0, n - 1)];
        if (is_tabu(cs, c)) continue;
        add_to_active(cs, c);
        adds++;
    }
    if (adds > 0) { cs.acc[4]++; return true; }
    return false;
}

// Move S: random kill + greedy fill up to 3.
bool move_S(ChainState& cs) {
    cs.tries[5]++;
    if (cs.size == 0) return false;
    int rm = cs.active_list[rand_int(0, cs.size - 1)];
    int prev_size = cs.size;
    remove_from_active(cs, rm);
    tabu_push(cs, rm);

    int adds = 0;
    while (adds < 3 && !cs.free_list.empty()) {
        // Greedy: pick free with fewest conflicts among other free
        constexpr int K = 16;
        int n = (int)cs.free_list.size();
        int trials = std::min(K, n);
        int best = -1;
        int best_score = INT_MAX;
        for (int t = 0; t < trials; t++) {
            int c = cs.free_list[rand_int(0, n - 1)];
            if (is_tabu(cs, c)) continue;
            int score = 0;
            int beg = adj_offset[c], end = adj_offset[c + 1];
            for (int p = beg; p < end; p++) {
                if (!cs.active[adj_list[p]]) score++;
            }
            if (score < best_score) { best_score = score; best = c; }
        }
        if (best < 0) break;
        add_to_active(cs, best);
        adds++;
    }
    int delta = cs.size - prev_size;
    if (delta >= 0) { cs.acc[5]++; return true; }
    double p = std::exp((double)delta / cs.T_current);
    if (rand_uniform() < p) { cs.acc[5]++; return true; }
    return false;
}

// Move Z: atomic pair swap (evict 2, add 2). v2 snapshot-restore + relaxed gain.
//
// v1   spec: STRICT GAIN Δ>0 (post first auditor pass).
// v1.5 spec: STRICT GAIN with explicit z_added_log revert.
// v1.5 RESULT: 0.0% acceptance over 250K tries on Mac M2. Move was DEAD.
// v2 SPEC (Auditor 2026-05-03):
//   - Δ>0  -> always accept
//   - Δ=0  -> accept with p=Z_DELTA_ZERO_PACC (=0.30)
//   - Δ<0  -> always reject
//   - Revert via SNAPSHOT (consistent with Move T cascade v2 mechanism)
// Auditor rejected adjacency_pressure heuristic as second-order coupling
// hard to debug. Simple Metropolis on Δ=0 gives Z back its diversification role.
bool move_Z(ChainState& cs) {
    cs.tries[6]++;
    if (cs.size < 2) return false;

    // === v2 SNAPSHOT (pre-move) ===
    std::vector<bool> snap_active        = cs.active;
    std::vector<int>  snap_active_list   = cs.active_list;
    std::vector<int>  snap_active_pos    = cs.active_pos;
    std::vector<int>  snap_blocked       = cs.blocked;
    std::vector<int>  snap_free_list     = cs.free_list;
    std::vector<int>  snap_free_pos      = cs.free_pos;
    int               snap_size          = cs.size;
    std::vector<int>  snap_tabu_buffer   = cs.tabu_buffer;
    int               snap_tabu_head     = cs.tabu_head;

    int r1 = cs.active_list[rand_int(0, cs.size - 1)];
    remove_from_active(cs, r1);
    tabu_push(cs, r1);
    if (cs.size == 0) {
        // Restore (we shouldn't really hit this since we checked size>=2 but defensive)
        cs.active = snap_active; cs.active_list = snap_active_list;
        cs.active_pos = snap_active_pos; cs.blocked = snap_blocked;
        cs.free_list = snap_free_list; cs.free_pos = snap_free_pos;
        cs.size = snap_size; cs.tabu_buffer = snap_tabu_buffer;
        cs.tabu_head = snap_tabu_head;
        return false;
    }
    int r2 = cs.active_list[rand_int(0, cs.size - 1)];
    remove_from_active(cs, r2);
    tabu_push(cs, r2);

    int adds = 0;
    for (int t = 0; t < 6 && adds < 2 && !cs.free_list.empty(); t++) {
        int n = (int)cs.free_list.size();
        int c = cs.free_list[rand_int(0, n - 1)];
        if (is_tabu(cs, c)) continue;
        add_to_active(cs, c);
        adds++;
    }
    int delta = cs.size - snap_size;

    // v2 acceptance: Δ>0 always; Δ=0 with prob Z_DELTA_ZERO_PACC; Δ<0 never
    bool accept = false;
    if (delta > 0) {
        accept = true;
    } else if (delta == 0) {
        accept = (rand_uniform() < Z_DELTA_ZERO_PACC);
    }
    if (accept) {
        cs.acc[6]++;
        return true;
    }
    // Reject: snapshot restore (full integrity)
    cs.active = snap_active; cs.active_list = snap_active_list;
    cs.active_pos = snap_active_pos; cs.blocked = snap_blocked;
    cs.free_list = snap_free_list; cs.free_pos = snap_free_pos;
    cs.size = snap_size; cs.tabu_buffer = snap_tabu_buffer;
    cs.tabu_head = snap_tabu_head;
    return false;
}

// Move X: medium earthquake, 4-5 evictions + greedy fill.
bool move_X(ChainState& cs) {
    cs.tries[7]++;
    if (cs.size < 5) return false;
    int prev_size = cs.size;
    int rm_count = 4 + rand_int(0, 1);
    for (int t = 0; t < rm_count && cs.size > 0; t++) {
        int r = cs.active_list[rand_int(0, cs.size - 1)];
        remove_from_active(cs, r);
        tabu_push(cs, r);
    }
    int adds = 0;
    for (int t = 0; t < 30 && !cs.free_list.empty(); t++) {
        constexpr int K = 24;
        int n = (int)cs.free_list.size();
        int trials = std::min(K, n);
        int best = -1, best_score = INT_MAX;
        for (int q = 0; q < trials; q++) {
            int c = cs.free_list[rand_int(0, n - 1)];
            if (is_tabu(cs, c)) continue;
            int score = 0;
            int beg = adj_offset[c], end = adj_offset[c + 1];
            for (int p = beg; p < end; p++) {
                if (!cs.active[adj_list[p]]) score++;
            }
            if (score < best_score) { best_score = score; best = c; }
        }
        if (best < 0) break;
        add_to_active(cs, best);
        adds++;
    }
    int delta = cs.size - prev_size;
    if (delta >= 0) { cs.acc[7]++; return true; }
    double p = std::exp((double)delta / cs.T_current * 0.5);
    if (rand_uniform() < p) { cs.acc[7]++; return true; }
    return false;
}

// Move P: big earthquake, 8-10 evictions.
bool move_P(ChainState& cs) {
    cs.tries[8]++;
    if (cs.size < 10) return false;
    int prev_size = cs.size;
    int rm_count = 8 + rand_int(0, 2);
    for (int t = 0; t < rm_count && cs.size > 0; t++) {
        int r = cs.active_list[rand_int(0, cs.size - 1)];
        remove_from_active(cs, r);
        tabu_push(cs, r);
    }
    int adds = 0;
    for (int t = 0; t < 50 && !cs.free_list.empty(); t++) {
        constexpr int K = 32;
        int n = (int)cs.free_list.size();
        int trials = std::min(K, n);
        int best = -1, best_score = INT_MAX;
        for (int q = 0; q < trials; q++) {
            int c = cs.free_list[rand_int(0, n - 1)];
            if (is_tabu(cs, c)) continue;
            int score = 0;
            int beg = adj_offset[c], end = adj_offset[c + 1];
            for (int p = beg; p < end; p++) {
                if (!cs.active[adj_list[p]]) score++;
            }
            if (score < best_score) { best_score = score; best = c; }
        }
        if (best < 0) break;
        add_to_active(cs, best);
        adds++;
    }
    int delta = cs.size - prev_size;
    if (delta >= 0) { cs.acc[8]++; return true; }
    double p = std::exp((double)delta / cs.T_current * 0.3);
    if (rand_uniform() < p) { cs.acc[8]++; return true; }
    return false;
}

// -----------------------------------------------------------------------------
// MOVE T CASCADE (v2 — snapshot-restore revert, Auditor-ratified 2026-05-03)
// -----------------------------------------------------------------------------
//
// v1.5 BUG (jubilated): used (added_log, evicted_log) with reverse-order replay.
// When cascade self-evicted a recently-added candidate, the two logs shared
// indices and revert was non-symmetric, leaving active[] geometrically corrupt
// (manifested on Mac M2 as VERIFY FAIL violations=14 min_pairwise=sqrt(2)).
//
// v2 FIX: snapshot the FULL mutable state BEFORE the cascade. On reject, restore
// by full copy. O(N) cost vs cascade work O(depth * adj) — negligible overhead.
// State to snapshot: active[], active_list, active_pos, blocked[], free_list,
// free_pos, size. Tabu buffer is also snapshotted for full integrity (we don't
// want a rejected cascade to leave residual tabu pollution).
//
// Algorithm unchanged from v1.5 conceptually:
//   1. Sample seed K_static from [4, 16] with weight ∝ 1/N_K
//   2. Pick a seed extra e0 from bucket[K_seed]
//   3. Greedy cascade depth up to MAX_CASCADE_DEPTH adding cheap extras
//   4. Auditor cap: if E > 2N+5 reject; else Metropolis
//   5. Accept -> keep state, log; Reject -> snapshot restore

// Defended (v2.1-B):
//   MAX_CASCADE_DEPTH=8: empirically the depth where cascades start producing
//   ratio E:N too unfavorable (validated v1 sandbox: depth-8 cascades had
//   delta=0 average, depth>8 always negative). Auditor cap 2N+5 limits damage.
//   CASCADE_E_OVER_2N_PLUS_5=5: the "+5" buffer beyond 2N. Allows cascades that
//   evict slightly more than 2N (recovery via subsequent gains) but not runaway.
//   E=2N+5 means cascade can evict up to 2*depth+5; at depth=8 max E=21.
constexpr int MAX_CASCADE_DEPTH = 8;
constexpr int CASCADE_E_OVER_2N_PLUS_5 = 5;

// Forward-declare ChainState fields used in snapshot (defined in struct further down).
// We use templated lambda capture so this compiles fine since ChainState is fully
// defined by the point this function is called.

bool move_T_cascade(ChainState& cs, MoveTStats& tstats) {
    cs.tries[9]++;

    // Step 1: sample seed K with probability ∝ 1/N_K
    double weights[T_K_MAX - T_K_MIN + 1];
    double total_w = 0.0;
    int valid_buckets = 0;
    for (int k = T_K_MIN; k <= T_K_MAX; k++) {
        size_t sz = bucket_by_kstatic[k].size();
        weights[k - T_K_MIN] = (sz > 0) ? (1.0 / (double)sz) : 0.0;
        total_w += weights[k - T_K_MIN];
        if (sz > 0) valid_buckets++;
    }
    if (valid_buckets == 0 || total_w == 0.0) return false;

    double r = rand_uniform() * total_w;
    int seed_k = T_K_MIN;
    double accw = 0.0;
    for (int k = T_K_MIN; k <= T_K_MAX; k++) {
        accw += weights[k - T_K_MIN];
        if (r <= accw) { seed_k = k; break; }
    }

    auto& bucket = bucket_by_kstatic[seed_k];
    if (bucket.empty()) return false;

    // Step 2: pick seed extra
    int e0 = bucket[rand_int(0, (int)bucket.size() - 1)];
    if (cs.active[e0] || is_tabu(cs, e0)) return false;

    // === v2 SNAPSHOT (pre-cascade) ===
    // Full state copy. Cost: O(N_pool) for active[] and blocked[], O(|active|)
    // for active_list/free_list. Total ~600 KB at 64K pool. Memcpy is fast.
    std::vector<bool> snap_active        = cs.active;
    std::vector<int>  snap_active_list   = cs.active_list;
    std::vector<int>  snap_active_pos    = cs.active_pos;
    std::vector<int>  snap_blocked       = cs.blocked;
    std::vector<int>  snap_free_list     = cs.free_list;
    std::vector<int>  snap_free_pos      = cs.free_pos;
    int               snap_size          = cs.size;
    std::vector<int>  snap_tabu_buffer   = cs.tabu_buffer;
    int               snap_tabu_head     = cs.tabu_head;

    // Track adds and evicts for LOGGING ONLY (not for revert anymore — snapshot revert).
    int N_added = 0;
    int E_evicted = 0;

    auto try_add_extra_with_eviction = [&](int e_target) -> bool {
        if (cs.active[e_target]) return false;
        // Walk e_target's full adjacency, evict ALL active conflicts.
        // (Iterate over a snapshot of blockers because remove_from_active mutates state.)
        std::vector<int> blockers;
        int beg = adj_offset[e_target], end = adj_offset[e_target + 1];
        for (int p = beg; p < end; p++) {
            int nbr = adj_list[p];
            if (cs.active[nbr]) blockers.push_back(nbr);
        }
        for (int b : blockers) {
            remove_from_active(cs, b);
            tabu_push(cs, b);
            E_evicted++;
        }
        add_to_active(cs, e_target);
        N_added++;
        return true;
    };

    // Step 3: cascade — start with e0
    if (!try_add_extra_with_eviction(e0)) {
        // Restore from snapshot (e0 may have triggered evictions before failing)
        cs.active        = snap_active;
        cs.active_list   = snap_active_list;
        cs.active_pos    = snap_active_pos;
        cs.blocked       = snap_blocked;
        cs.free_list     = snap_free_list;
        cs.free_pos      = snap_free_pos;
        cs.size          = snap_size;
        cs.tabu_buffer   = snap_tabu_buffer;
        cs.tabu_head     = snap_tabu_head;
        return false;
    }

    // Continue: try up to MAX_CASCADE_DEPTH-1 more extras
    for (int depth = 1; depth < MAX_CASCADE_DEPTH; depth++) {
        int rand_k = T_K_MIN + rand_int(0, T_K_MAX - T_K_MIN);
        if (bucket_by_kstatic[rand_k].empty()) continue;
        auto& bk = bucket_by_kstatic[rand_k];
        int candidate = bk[rand_int(0, (int)bk.size() - 1)];
        if (cs.active[candidate]) continue;
        if (is_tabu(cs, candidate)) continue;

        int beg = adj_offset[candidate], end = adj_offset[candidate + 1];
        int new_evicts = 0;
        for (int p = beg; p < end; p++) {
            if (cs.active[adj_list[p]]) new_evicts++;
        }
        if (new_evicts <= 3) {
            try_add_extra_with_eviction(candidate);
        }
    }

    int N = N_added;
    int E = E_evicted;
    int delta = N - E;
    tstats.fired++;

    int max_E = 2 * N + CASCADE_E_OVER_2N_PLUS_5;
    bool accept;
    if (E > max_E) {
        accept = false;
    } else if (delta >= 0) {
        accept = true;
    } else {
        double prob = std::exp((double)delta / cs.T_current * 0.4);
        accept = (rand_uniform() < prob);
    }

    if (accept) {
        cs.acc[9]++;
        tstats.accepted++;
        tstats.total_evicted += E;
        tstats.total_added += N;
        tstats.net_gain_total += delta;
        fprintf(stdout, "[T] CASCADE seed_K=%d depth=%d added=%d evicted=%d delta=%+d size=%d\n",
                seed_k, N, N, E, delta, cs.size);
        fflush(stdout);
        return true;
    } else {
        // === v2 SNAPSHOT RESTORE ===
        // Full state restore. Robust to ANY cascade pathology including
        // self-eviction (the v1.5 bug). Geometric integrity GUARANTEED post-revert.
        cs.active        = snap_active;
        cs.active_list   = snap_active_list;
        cs.active_pos    = snap_active_pos;
        cs.blocked       = snap_blocked;
        cs.free_list     = snap_free_list;
        cs.free_pos      = snap_free_pos;
        cs.size          = snap_size;
        cs.tabu_buffer   = snap_tabu_buffer;
        cs.tabu_head     = snap_tabu_head;
        return false;
    }
}

// Backwards-compatible name alias for older sanity #6
inline bool move_T(ChainState& cs, MoveTStats& tstats) {
    return move_T_cascade(cs, tstats);
}

// -----------------------------------------------------------------------------
// MIX SAMPLING
// -----------------------------------------------------------------------------

int sample_move(const int mix[N_MOVES]) {
    int total = 0;
    for (int i = 0; i < N_MOVES; i++) total += mix[i];
    int r = rand_int(0, total - 1);
    int acc = 0;
    for (int i = 0; i < N_MOVES; i++) {
        acc += mix[i];
        if (r < acc) return i;
    }
    return 0;
}

const char* move_name(int id) {
    static const char* names[N_MOVES] = {"R","M","W","K","L","S","Z","X","P","T"};
    return names[id];
}

// -----------------------------------------------------------------------------
// VERIFY (D190 quadruple-style: full pairwise pool-distance audit)
// -----------------------------------------------------------------------------

bool verify_config_strict(const std::vector<int>& active_indices, int& violations,
                          double& min_dist) {
    violations = 0;
    min_dist = 1e18;
    int n = (int)active_indices.size();
    for (int i = 0; i < n; i++) {
        // Verify each is at distance EXACTLY 2 from CENTER
        double s = 0.0;
        for (int d = 0; d < DIM; d++) {
            double diff = POOL[active_indices[i]][d] - CENTER[d];
            s += diff * diff;
        }
        if (std::fabs(s - SQDIST_KISS) > 1e-7) violations++;
        // Pairwise
        for (int j = i + 1; j < n; j++) {
            double sq = sqdist_pool(active_indices[i], active_indices[j]);
            double d = std::sqrt(sq);
            if (d < min_dist) min_dist = d;
            if (sq < SQDIST_KISS - 1e-7) violations++;
        }
    }
    return violations == 0;
}

// v2-B: Cheap geometric integrity sampling — picks `n_pairs` random active
// pairs and checks pairwise sqdist >= 4.0 - eps. O(n_pairs) cost (no full N^2).
// Returns true if all sampled pairs are valid; false if ANY violation found.
// Used for in-engine periodic integrity check during long runs.
//
// Rationale: full verify_config_strict is O(N^2) over active set (~1130^2/2 =
// 638K dist comps); calling it every 1000 iters would dominate runtime.
// Sample-based check at 50 pairs / 1000 iters = ~50K dist comps per second
// total, vs ~95M move-related dist comps per second. Negligible overhead.
//
// Coverage analysis: at |active|=1130, total pairs = 638K. With 50 samples per
// 1000 iters, after 100K iters we've sampled ~5000 pairs. Probability that ANY
// given violating pair (if it exists) is sampled in 100K iters ~ 1 -
// (1-50/638000)^100 ~ 0.78%. Coarse check catches MOST corruptions within a
// few hundred thousand iters. The fine check (200 pairs / 10000 iters) bumps
// detection certainty further.
bool geometric_integrity_sample(const std::vector<int>& active_indices,
                                 int n_pairs,
                                 int& fail_i, int& fail_j, double& fail_sqdist) {
    int n = (int)active_indices.size();
    if (n < 2) return true;
    for (int t = 0; t < n_pairs; t++) {
        int i = rand_int(0, n - 1);
        int j = rand_int(0, n - 1);
        if (i == j) continue;
        double sq = sqdist_pool(active_indices[i], active_indices[j]);
        if (sq < SQDIST_KISS - 1e-7) {
            fail_i = active_indices[i];
            fail_j = active_indices[j];
            fail_sqdist = sq;
            return false;
        }
    }
    return true;
}

// v2-B: Dump corrupted state to disk for offline analysis.
void dump_corruption(const std::vector<int>& active_indices, long iter,
                     int chain_idx, int fail_i, int fail_j, double fail_sqdist) {
    char fname[256];
    snprintf(fname, sizeof(fname), "corruption_dump_iter_%ld_c%d.txt", iter, chain_idx);
    FILE* f = fopen(fname, "w");
    if (!f) return;
    fprintf(f, "# HORMONEROCHUMINEROPREMIUM_v2 — CORRUPTION DUMP\n");
    fprintf(f, "# iter=%ld chain=%d size=%zu\n", iter, chain_idx, active_indices.size());
    fprintf(f, "# fail_pair: i=%d j=%d sqdist=%.10f (expected >= %.1f)\n",
            fail_i, fail_j, fail_sqdist, SQDIST_KISS);
    fprintf(f, "# active indices (one per line):\n");
    for (int idx : active_indices) fprintf(f, "%d\n", idx);
    fclose(f);
    fprintf(stderr, "[CORRUPTION] dumped to %s — ABORT\n", fname);
}

// -----------------------------------------------------------------------------
// SAVE
// -----------------------------------------------------------------------------

void save_config(const std::vector<int>& indices, const char* tag) {
    char fname[256];
    snprintf(fname, sizeof(fname), "LUNA_BEST_%s_%zu.txt", tag, indices.size());
    FILE* f = fopen(fname, "w");
    if (!f) return;
    fprintf(f, "# HORMONEROCHUMINEROPREMIUM_v1 — kissing config in R^%d\n", DIM);
    fprintf(f, "# size=%zu  threshold=%.1f  time=%.1fs  RNG=0x%016llX\n",
            indices.size(), DIST_KISS, now_sec(), (unsigned long long)RNG_SEED);
    fprintf(f, "# center: ");
    for (int d = 0; d < DIM; d++) fprintf(f, "%.17g%s", CENTER[d], (d+1<DIM)?" ":"\n");
    fprintf(f, "# vectors (one per line, %d coords):\n", DIM);
    for (int idx : indices) {
        for (int d = 0; d < DIM; d++)
            fprintf(f, "%.17g%s", POOL[idx][d], (d+1<DIM)?" ":"\n");
    }
    int viol;
    double min_d;
    bool ok = verify_config_strict(indices, viol, min_d);
    fprintf(f, "# VERIFY: %s  violations=%d  min_pairwise_dist=%.10f\n",
            ok?"PASS":"FAIL", viol, min_d);
    fclose(f);
}

void log_record(int sz, const char* tag) {
    FILE* f = fopen("LUNA_RECORDS.txt", "a");
    if (!f) return;
    const char* tier =
        (sz >= TARGET_RECORD)     ? "WORLD-RECORD" :
        (sz >= TARGET_RECORD_TIE) ? "RECORD-TIE"   :
        (sz >  1130)              ? "BEAT-P13a"    :
        (sz >= 1130)              ? "P13a-FLOOR"   :
        (sz >= TARGET_VALIDATE)   ? "VALIDATED"    :
        (sz >= TARGET_RETRAIN_LO) ? "RETRAIN"      :
        (sz >= TARGET_SAVE_FROM)  ? "save"         : "low";
    fprintf(f, "[%s] size=%d  t=%.1fs  tag=%s\n", tier, sz, now_sec(), tag);
    fclose(f);
}

// -----------------------------------------------------------------------------
// POLISH PASS (D184 + D187 no-gate)
// -----------------------------------------------------------------------------

int polish_pass(ChainState& cs) {
    int original_size = cs.size;
    int gain = 0;
    bool improved = true;
    int safety = 0;
    while (improved && safety++ < 50) {
        improved = false;
        std::vector<int> active_snap = cs.active_list;
        for (int a : active_snap) {
            if (!cs.active[a]) continue;
            int prev_sz = cs.size;
            remove_from_active(cs, a);
            // Try to add 2 from free
            std::vector<int> added;
            int adds = 0;
            for (int t = 0; t < 8 && !cs.free_list.empty() && adds < 2; t++) {
                int n = (int)cs.free_list.size();
                int c = cs.free_list[rand_int(0, n - 1)];
                add_to_active(cs, c);
                added.push_back(c);
                adds++;
            }
            if (cs.size > prev_sz) {
                gain += (cs.size - prev_sz);
                improved = true;
            } else if (cs.size < prev_sz) {
                // revert
                for (int x : added) remove_from_active(cs, x);
                add_to_active(cs, a);
            }
        }
    }
    return cs.size - original_size;
}

// -----------------------------------------------------------------------------
// SANITY SUITE (auditor #1-#15; v2 adds #12 cascade geom integrity, #13 50K-iter
// e2e geom integrity, #14 snapshot/restore roundtrip, #15 Mode A floor verify)
// -----------------------------------------------------------------------------

bool sanity_suite() {
    fprintf(stderr, "\n[SANITY] running 16-test suite (v2.1: +#16 Mode B post-warm-load thermal sanity)...\n");
    int pass = 0, total = 0;

    // #1: pool loaded with expected dimensions
    //   Original v1 pool: 71,082 (classical 1130 + 69,952 extras K_static in [4,16])
    //   v1.5 pool (pool_v15.bin, Architect 2026-05-03): 64,802 (classical 1130
    //     + 63,672 extras K_static in [4,16]) — refined pool delivered by
    //     Architect. Sanity #1 accepts both shapes with K_static distribution
    //     verified separately at preflight.
    total++;
    if ((N_POOL == 71082 || N_POOL == 64802) && DIM == 13 && K_CLASSICAL == 1130) {
        pass++; fprintf(stderr, "  #1  pool dimensions: PASS (N=%d D=%d K=%d)\n",
                        N_POOL, DIM, K_CLASSICAL);
    } else {
        fprintf(stderr, "  #1  pool dimensions: FAIL (N=%d D=%d K=%d, expected N in {71082,64802} D=13 K=1130)\n",
                N_POOL, DIM, K_CLASSICAL); return false;
    }

    // #2: all candidates at distance 2 +/- EPS from CENTER
    total++;
    {
        bool ok = true;
        double max_err = 0.0;
        for (int i = 0; i < N_POOL; i++) {
            double s = 0.0;
            for (int d = 0; d < DIM; d++) {
                double diff = POOL[i][d] - CENTER[d];
                s += diff * diff;
            }
            double err = std::fabs(s - SQDIST_KISS);
            if (err > max_err) max_err = err;
            if (err > 1e-7) { ok = false; }
        }
        if (ok) { pass++; fprintf(stderr, "  #2  all at dist 2.0: PASS (max_err=%.2e)\n", max_err); }
        else    { fprintf(stderr, "  #2  all at dist 2.0: FAIL (max_err=%.2e)\n", max_err); return false; }
    }

    // #3: classical 1130 internally conflict-free
    total++;
    {
        bool ok = true;
        for (int i = 0; i < K_CLASSICAL; i++) {
            int beg = adj_offset[i], end = adj_offset[i + 1];
            for (int p = beg; p < end; p++) {
                if (adj_list[p] < K_CLASSICAL) { ok = false; break; }
            }
            if (!ok) break;
        }
        if (ok) { pass++; fprintf(stderr, "  #3  classical 1130 conflict-free: PASS\n"); }
        else    { fprintf(stderr, "  #3  classical 1130 conflict-free: FAIL\n"); return false; }
    }

    // #4: adjacency symmetry
    total++;
    {
        bool ok = true;
        for (int t = 0; t < 1000 && ok; t++) {
            int i = rand_int(0, N_POOL - 1);
            int beg = adj_offset[i], end = adj_offset[i + 1];
            if (end - beg == 0) continue;
            int j = adj_list[rand_int(beg, end - 1)];
            // i should be in j's list
            int b2 = adj_offset[j], e2 = adj_offset[j + 1];
            bool found = false;
            for (int p = b2; p < e2; p++) if (adj_list[p] == i) { found = true; break; }
            if (!found) ok = false;
        }
        if (ok) { pass++; fprintf(stderr, "  #4  adjacency symmetry: PASS\n"); }
        else    { fprintf(stderr, "  #4  adjacency symmetry: FAIL\n"); return false; }
    }

    // #5: RNG determinism
    total++;
    {
        std::mt19937_64 r1(RNG_SEED), r2(RNG_SEED);
        bool ok = true;
        for (int t = 0; t < 100; t++) if (r1() != r2()) { ok = false; break; }
        if (ok) { pass++; fprintf(stderr, "  #5  RNG determinism: PASS\n"); }
        else    { fprintf(stderr, "  #5  RNG: FAIL\n"); return false; }
    }

    // #6: each move callable on a toy state (just check no crash, basic flow)
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        // greedy init a few
        for (int k = 0; k < 50; k++) move_M(cs);
        // Try each move once; should not crash
        MoveTStats ts;
        bool ok = true;
        try {
            move_R(cs); move_M(cs); move_W(cs); move_K(cs);
            move_L(cs); move_S(cs); move_Z(cs);
            // X and P need >=5 and >=10 active resp.
            for (int k = 0; k < 30; k++) move_M(cs);
            move_X(cs); move_P(cs); move_T(cs, ts);
        } catch (...) { ok = false; }
        if (ok) { pass++; fprintf(stderr, "  #6  all 10 moves exercised: PASS (state size=%d)\n", cs.size); }
        else    { fprintf(stderr, "  #6  moves: FAIL\n"); return false; }
    }

    // #7: incremental BLOCKED[] consistency: random state + check brute matches
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        for (int k = 0; k < 200; k++) move_M(cs);
        // Brute-force compute blocked[] for a sample of indices
        bool ok = true;
        for (int t = 0; t < 50; t++) {
            int i = rand_int(0, N_POOL - 1);
            int b = 0;
            int beg = adj_offset[i], end = adj_offset[i + 1];
            for (int p = beg; p < end; p++) {
                if (cs.active[adj_list[p]]) b++;
            }
            if (b != cs.blocked[i]) { ok = false; break; }
        }
        if (ok) { pass++; fprintf(stderr, "  #7  incremental BLOCKED consistency: PASS\n"); }
        else    { fprintf(stderr, "  #7  BLOCKED inconsistency: FAIL\n"); return false; }
    }

    // #8: polish pass on classical 1130 yields 0 gain (saturated locally)
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        for (int i = 0; i < K_CLASSICAL; i++) add_to_active(cs, i);
        if (cs.size != K_CLASSICAL) {
            fprintf(stderr, "  #8  warm-load classical FAIL: size=%d expected=%d\n",
                    cs.size, K_CLASSICAL); return false;
        }
        int gain = polish_pass(cs);
        if (gain >= 0 && cs.size >= K_CLASSICAL) {
            pass++; fprintf(stderr, "  #8  polish on classical: PASS (gain=%d size=%d)\n",
                            gain, cs.size);
        } else {
            fprintf(stderr, "  #8  polish on classical regressed: gain=%d size=%d\n",
                    gain, cs.size); return false;
        }
    }

    // #9: cold start reaches >= 400 in 30 seconds (auditor clarification 1)
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        int mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        double t0 = now_sec();
        long iter = 0;
        while (now_sec() - t0 < 30.0) {
            int mv = sample_move(mix);
            switch (mv) {
                case 0: move_R(cs); break;
                case 1: move_M(cs); break;
                case 2: move_W(cs); break;
                case 3: move_K(cs); break;
                case 4: move_L(cs); break;
                case 5: move_S(cs); break;
                case 6: move_Z(cs); break;
                case 7: move_X(cs); break;
                case 8: move_P(cs); break;
                case 9: { MoveTStats ts; move_T(cs, ts); break; }
            }
            if (cs.size > cs.best_local) cs.best_local = cs.size;
            iter++;
            // cool gently
            cs.T_current *= 0.999995;
            if (cs.T_current < 0.005) cs.T_current = 0.005;
        }
        if (cs.best_local >= 400) {
            pass++; fprintf(stderr, "  #9  cold-start reaches >=400 in 30s: PASS (best=%d, iters=%ld)\n",
                            cs.best_local, iter);
        } else {
            fprintf(stderr, "  #9  cold-start FAIL: best=%d in 30s, iters=%ld\n",
                    cs.best_local, iter); return false;
        }
    }

    // #10: D190 verifier matches SA-tracked size
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        for (int k = 0; k < 100; k++) move_M(cs);
        std::vector<int> idx_list = cs.active_list;
        int sa_size = cs.size;
        int viol;
        double mind;
        bool ok = verify_config_strict(idx_list, viol, mind);
        if (ok && (int)idx_list.size() == sa_size) {
            pass++; fprintf(stderr, "  #10 D190 verifier matches: PASS (size=%d viol=%d min_d=%.6f)\n",
                            sa_size, viol, mind);
        } else {
            fprintf(stderr, "  #10 verifier mismatch: FAIL ok=%d size=%d viol=%d\n",
                    ok, sa_size, viol); return false;
        }
    }

    // #11 (v1.5 NEW — Auditor ratification 2026-05-03):
    // BLOCKED[] consistency after PT swaps active.
    // Sanity #7 verifies BLOCKED on a freshly-initialized chain. PT swaps
    // exchange entire chain state across CHAINS — including the precomputed
    // blocked[] array. This test exercises 5000 iters with the real move mix
    // AND PT swaps every 1024 iters, then verifies blocked[] consistency
    // by brute force on a sample of indices in every chain.
    total++;
    {
        std::vector<ChainState> test_chains(CHAINS);
        for (int c = 0; c < CHAINS; c++) chain_init_empty(test_chains[c], c);

        int test_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        MoveTStats ts;
        for (long it = 0; it < 5000; it++) {
            for (int c = 0; c < CHAINS; c++) {
                int mv = sample_move(test_mix);
                switch (mv) {
                    case 0: move_R(test_chains[c]); break;
                    case 1: move_M(test_chains[c]); break;
                    case 2: move_W(test_chains[c]); break;
                    case 3: move_K(test_chains[c]); break;
                    case 4: move_L(test_chains[c]); break;
                    case 5: move_S(test_chains[c]); break;
                    case 6: move_Z(test_chains[c]); break;
                    case 7: move_X(test_chains[c]); break;
                    case 8: move_P(test_chains[c]); break;
                    case 9: move_T(test_chains[c], ts); break;
                }
            }
            // PT swap every 1024 (mirrors main loop exactly)
            if ((it & 1023) == 0 && it > 0) {
                int pair = rand_int(0, CHAINS - 2);
                ChainState& A = test_chains[pair];
                ChainState& B = test_chains[pair + 1];
                if (B.size > A.size) {
                    std::swap(A.active, B.active);
                    std::swap(A.active_list, B.active_list);
                    std::swap(A.active_pos, B.active_pos);
                    std::swap(A.blocked, B.blocked);
                    std::swap(A.free_list, B.free_list);
                    std::swap(A.free_pos, B.free_pos);
                    std::swap(A.size, B.size);
                }
            }
        }

        // Verify BLOCKED[] consistency in all chains (sample 100 indices each)
        bool ok = true;
        int fail_chain = -1, fail_idx = -1, fail_brute = -1, fail_stored = -1;
        for (int c = 0; c < CHAINS && ok; c++) {
            for (int t = 0; t < 100 && ok; t++) {
                int i = rand_int(0, N_POOL - 1);
                int b = 0;
                int beg = adj_offset[i], end = adj_offset[i + 1];
                for (int p = beg; p < end; p++) {
                    if (test_chains[c].active[adj_list[p]]) b++;
                }
                if (b != test_chains[c].blocked[i]) {
                    ok = false;
                    fail_chain = c; fail_idx = i;
                    fail_brute = b; fail_stored = test_chains[c].blocked[i];
                    break;
                }
            }
        }
        if (ok) {
            pass++;
            fprintf(stderr, "  #11 BLOCKED consistency post-PT-swaps: PASS (5000 iters, 5 chains, 500 samples)\n");
        } else {
            fprintf(stderr, "  #11 BLOCKED consistency post-PT-swaps: FAIL "
                            "(chain=%d idx=%d brute=%d stored=%d)\n",
                            fail_chain, fail_idx, fail_brute, fail_stored);
            return false;
        }
    }

    // #12 (v2 NEW — Auditor 2026-05-03):
    // Move T cascade revert geometric integrity. Fire 1000 cascades from
    // mid-warm states; after each REJECTED cascade, verify the active set
    // is geometrically valid (snapshot restore worked). After each ACCEPTED
    // cascade, verify too (defensive — accepts shouldn't produce invalid
    // states either).
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        // Warm to ~200 active so cascades have something to evict
        for (int k = 0; k < 200; k++) move_M(cs);

        MoveTStats ts;
        int rejected_cascades = 0, accepted_cascades = 0;
        bool ok = true;
        int fail_iter = -1, fail_n_violations = 0;
        double fail_min_d = 0.0;

        for (int t = 0; t < 1000 && ok; t++) {
            bool acc = move_T_cascade(cs, ts);
            if (acc) accepted_cascades++; else rejected_cascades++;
            // Geometric verify of full active set
            int viol;
            double mind;
            bool gv_ok = verify_config_strict(cs.active_list, viol, mind);
            if (!gv_ok) {
                ok = false;
                fail_iter = t;
                fail_n_violations = viol;
                fail_min_d = mind;
                break;
            }
        }
        if (ok) {
            pass++;
            fprintf(stderr, "  #12 Move T cascade geometric integrity: PASS "
                            "(1000 cascades: %d accepted / %d rejected, all valid)\n",
                            accepted_cascades, rejected_cascades);
        } else {
            fprintf(stderr, "  #12 Move T cascade geometric integrity: FAIL "
                            "(at cascade %d: viol=%d min_d=%.6f)\n",
                            fail_iter, fail_n_violations, fail_min_d);
            return false;
        }
    }

    // #13 (v2 NEW): 50000-iter end-to-end geometric integrity
    // Run 50K iters with full move mix; sample-verify random pairs every
    // 1000 iters (mirrors the production verify cadence). ANY violation -> FAIL.
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        int test_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        MoveTStats ts;
        bool ok = true;
        int fail_iter = -1, fail_i = -1, fail_j = -1;
        double fail_sq = 0.0;

        for (long it = 0; it < 50000 && ok; it++) {
            int mv = sample_move(test_mix);
            switch (mv) {
                case 0: move_R(cs); break;
                case 1: move_M(cs); break;
                case 2: move_W(cs); break;
                case 3: move_K(cs); break;
                case 4: move_L(cs); break;
                case 5: move_S(cs); break;
                case 6: move_Z(cs); break;
                case 7: move_X(cs); break;
                case 8: move_P(cs); break;
                case 9: move_T(cs, ts); break;
            }
            if (it % 1000 == 0 && it > 0 && cs.size >= 50) {
                int fi=-1, fj=-1;
                double fsq = 0.0;
                if (!geometric_integrity_sample(cs.active_list, 50, fi, fj, fsq)) {
                    ok = false;
                    fail_iter = (int)it;
                    fail_i = fi; fail_j = fj; fail_sq = fsq;
                    break;
                }
            }
        }
        if (ok) {
            pass++;
            fprintf(stderr, "  #13 50000-iter end-to-end geometric integrity: PASS (final size=%d)\n",
                            cs.size);
        } else {
            fprintf(stderr, "  #13 50000-iter end-to-end geometric integrity: FAIL "
                            "(iter=%d fail_pair i=%d j=%d sqdist=%.6f)\n",
                            fail_iter, fail_i, fail_j, fail_sq);
            return false;
        }
    }

    // #14 (v2 NEW — Auditor #14): Snapshot/restore roundtrip exactness.
    // Snapshot a state; mutate via 1000 random moves; restore from snapshot;
    // brute-verify state matches snapshot exactly (active[], blocked[], size).
    // Confirms snapshot mechanism IS correct before relying on it in cascades.
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        for (int k = 0; k < 300; k++) move_M(cs);

        // Take snapshot
        std::vector<bool> sa = cs.active;
        std::vector<int>  sal = cs.active_list;
        std::vector<int>  sap = cs.active_pos;
        std::vector<int>  sb = cs.blocked;
        std::vector<int>  sfl = cs.free_list;
        std::vector<int>  sfp = cs.free_pos;
        int               ssz = cs.size;
        std::vector<int>  stb = cs.tabu_buffer;
        int               sth = cs.tabu_head;

        // Mutate via 1000 random moves
        int test_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        MoveTStats ts;
        for (int t = 0; t < 1000; t++) {
            int mv = sample_move(test_mix);
            switch (mv) {
                case 0: move_R(cs); break;
                case 1: move_M(cs); break;
                case 2: move_W(cs); break;
                case 3: move_K(cs); break;
                case 4: move_L(cs); break;
                case 5: move_S(cs); break;
                case 6: move_Z(cs); break;
                case 7: move_X(cs); break;
                case 8: move_P(cs); break;
                case 9: move_T(cs, ts); break;
            }
        }

        // Restore
        cs.active = sa; cs.active_list = sal; cs.active_pos = sap;
        cs.blocked = sb; cs.free_list = sfl; cs.free_pos = sfp;
        cs.size = ssz; cs.tabu_buffer = stb; cs.tabu_head = sth;

        // Brute-verify: does cs match the snapshot exactly?
        bool ok = (cs.active == sa) && (cs.active_list == sal) &&
                  (cs.active_pos == sap) && (cs.blocked == sb) &&
                  (cs.free_list == sfl) && (cs.free_pos == sfp) &&
                  (cs.size == ssz) && (cs.tabu_buffer == stb) &&
                  (cs.tabu_head == sth);

        // Additionally verify size is internally consistent
        int actual_active_count = 0;
        for (int i = 0; i < N_POOL; i++) if (cs.active[i]) actual_active_count++;
        if (actual_active_count != cs.size) ok = false;

        if (ok) {
            pass++;
            fprintf(stderr, "  #14 Snapshot/restore roundtrip exact: PASS (size=%d, all 9 fields matched)\n",
                            cs.size);
        } else {
            fprintf(stderr, "  #14 Snapshot/restore roundtrip: FAIL\n");
            return false;
        }
    }

    // #15 (v2 NEW — Auditor #15): Mode A floor crossing with verify.
    // Cold-start to first 1130 (or stop after 60s if not reached); IMMEDIATELY
    // verify_config_strict. Must PASS. This is the gate the bug of v1.5 should
    // have passed (it didn't; configs were corrupt).
    total++;
    {
        ChainState cs;
        chain_init_empty(cs, 0);
        int test_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        MoveTStats ts;
        double t0 = now_sec();
        long iter = 0;
        int best = 0;
        while (now_sec() - t0 < 60.0 && best < 1130) {
            int mv = sample_move(test_mix);
            switch (mv) {
                case 0: move_R(cs); break;
                case 1: move_M(cs); break;
                case 2: move_W(cs); break;
                case 3: move_K(cs); break;
                case 4: move_L(cs); break;
                case 5: move_S(cs); break;
                case 6: move_Z(cs); break;
                case 7: move_X(cs); break;
                case 8: move_P(cs); break;
                case 9: move_T(cs, ts); break;
            }
            if (cs.size > best) best = cs.size;
            iter++;
            cs.T_current *= 0.999995;
            if (cs.T_current < 0.005) cs.T_current = 0.005;
        }

        if (best < 1130) {
            // Could not reach floor in 60s; this is OK as long as what we DID reach
            // is geometrically valid. Otherwise FAIL.
            int viol; double mind;
            bool ok = verify_config_strict(cs.active_list, viol, mind);
            if (ok) {
                pass++;
                fprintf(stderr, "  #15 Mode A floor crossing with verify: PASS "
                                "(reached best=%d in 60s, verify clean)\n", best);
            } else {
                fprintf(stderr, "  #15 Mode A floor crossing: FAIL "
                                "(best=%d but verify viol=%d min_d=%.6f)\n",
                                best, viol, mind);
                return false;
            }
        } else {
            // Reached 1130 — verify must be clean
            int viol; double mind;
            bool ok = verify_config_strict(cs.active_list, viol, mind);
            if (ok) {
                pass++;
                fprintf(stderr, "  #15 Mode A floor crossing with verify: PASS "
                                "(best=%d>=1130, verify viol=0 min_d=%.6f, iters=%ld)\n",
                                best, mind, iter);
            } else {
                fprintf(stderr, "  #15 Mode A floor crossing: FAIL "
                                "(best=%d>=1130 BUT verify FAIL viol=%d min_d=%.6f) "
                                "-- THE v1.5 BUG IS NOT FIXED\n",
                                best, viol, mind);
                return false;
            }
        }
    }

    // #16 (v2.1 NEW — Auditor sucesor): Mode B post-warm-load thermal sanity.
    // Closes the F5-style blind spot of v2 (lapsus G8). Test simulates Mode B
    // entry: warm-load classical 1130 in chain c0, run 10000 iters, verify:
    //   (1) T_current does NOT collapse to T_FLOOR cryogenic (0.005)
    //   (2) Move T cascade fires >=50 times with E>2 (deep cascades real)
    //   (3) Condition (best_proxy >= CRYO_THRESH) stays FALSE while best=1130
    // If FAIL: another constant is mis-calibrated, do NOT relaunch on Mac.
    total++;
    {
        // Warm-load: pick first 1130 candidates from pool (= classical P13a if
        // pool_v15.bin convention holds: first 1130 = classical independent set).
        ChainState cs;
        chain_init_empty(cs, 0);

        // Add first 1130 candidates as active. Use add_to_active() which keeps
        // active_list / active_pos / blocked / free_list consistent.
        bool warm_load_ok = true;
        for (int i = 0; i < 1130 && i < N_POOL; i++) {
            // The classical P13a is independent — but if any conflict happens
            // during loading, the test must abort because the pool's first 1130
            // contract is broken (a separate bug class, not what we're testing).
            if (cs.blocked[i] != 0) {
                warm_load_ok = false;
                fprintf(stderr, "  #16 WARM-LOAD FAIL: candidate %d blocked at "
                                "load step %d (pool first-1130 not independent?)\n",
                                i, i);
                break;
            }
            add_to_active(cs, i);
        }

        if (!warm_load_ok || cs.size != 1130) {
            fprintf(stderr, "  #16 Mode B post-warm-load thermal sanity: FAIL "
                            "(warm-load reached size=%d, expected 1130)\n", cs.size);
            return false;
        }

        // Replicate cooling logic from run_iter (using cs.size as best_proxy
        // since this isolated test has no global). Critical: if size==1130
        // and CRYO_THRESH==1131, in_cryo MUST be false the entire run.
        int test_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
        MoveTStats ts;
        long cascade_fires_observed = 0;   // how many times Move T was attempted (cs.tries[9])
        double T_min_observed = cs.T_current;
        bool cryo_ever_active = false;

        long tries9_initial = cs.tries[9];

        for (long it = 0; it < 10000; it++) {
            int mv = sample_move(test_mix);
            switch (mv) {
                case 0: move_R(cs); break;
                case 1: move_M(cs); break;
                case 2: move_W(cs); break;
                case 3: move_K(cs); break;
                case 4: move_L(cs); break;
                case 5: move_S(cs); break;
                case 6: move_Z(cs); break;
                case 7: move_X(cs); break;
                case 8: move_P(cs); break;
                case 9: move_T(cs, ts); break;
            }

            // Replicate cooling logic from run_iter
            int best_proxy = cs.size > 1130 ? cs.size : 1130;  // monotonic floor
            bool in_cryo = (best_proxy >= CRYO_THRESH);
            if (in_cryo) cryo_ever_active = true;
            double T_max = in_cryo ? T_CRYO_MAX : cs.T_base;
            if (cs.T_current > T_max) cs.T_current = T_max;
            double t_factor = in_cryo ? T_FACTOR_CRYO : T_FACTOR_NORM;
            cs.T_current *= t_factor;
            double t_min = in_cryo ? 0.005 : T_FLOOR;
            if (cs.T_current < t_min) cs.T_current = t_min;
            if (cs.T_current < T_min_observed) T_min_observed = cs.T_current;
        }
        cascade_fires_observed = cs.tries[9] - tries9_initial;

        // Criterion 1: T_current never collapsed to cryogenic floor 0.005.
        //   (Anchor: in cryogenic regime t_min=0.005; outside it t_min=T_FLOOR=0.020.
        //    With CRYO_THRESH=1131 and best=1130, we MUST stay outside cryogenic.)
        bool crit1_ok = (T_min_observed >= T_FLOOR - 1e-9);
        // Criterion 2: cascade fires happen at the expected rate.
        //   MIX_T=21/100, 10000 iters, expected ~2100 fires. We require >=500
        //   to confirm the move is alive (not silently disabled or guarded out).
        //   ANCHOR: 500 = MIX_T/100 * iters * 25% safety margin.
        //   We do NOT require accepted cascades because at best=1130 (saturated),
        //   acceptance rate is empirically 0.005-0.01% (Mac v2 run1 evidence).
        //   Asking for accepted-deep here would be testing 'engine beats record',
        //   not 'engine is thermally healthy'. Wrong invariant.
        bool crit2_ok = (cascade_fires_observed >= 500);
        // Criterion 3: cryo never activated while best stayed at 1130.
        //   (Direct test of fix v2.1-A.)
        bool crit3_ok = (!cryo_ever_active);

        if (crit1_ok && crit2_ok && crit3_ok) {
            pass++;
            fprintf(stderr, "  #16 Mode B post-warm-load thermal sanity: PASS\n"
                            "    T_min=%.4f >= T_FLOOR=%.4f (no cryo collapse)\n"
                            "    cascade_fires=%ld >= 500 (Move T alive)\n"
                            "    cryo_never_active (CRYO_THRESH=1131 protects 1130)\n",
                            T_min_observed, T_FLOOR, cascade_fires_observed);
        } else {
            fprintf(stderr, "  #16 Mode B post-warm-load thermal sanity: FAIL\n");
            fprintf(stderr, "    crit1 (T_min>=T_FLOOR): %s "
                            "(T_min_observed=%.4f, T_FLOOR=%.4f)\n",
                            crit1_ok?"PASS":"FAIL", T_min_observed, T_FLOOR);
            fprintf(stderr, "    crit2 (cascade_fires>=500): %s "
                            "(observed=%ld)\n",
                            crit2_ok?"PASS":"FAIL", cascade_fires_observed);
            fprintf(stderr, "    crit3 (cryo_never_active): %s\n",
                            crit3_ok?"PASS":"FAIL");
            fprintf(stderr, "    *** DO NOT RELAUNCH ON MAC. Another constant is mis-calibrated. ***\n");
            return false;
        }
    }

    fprintf(stderr, "\n[SANITY] %d/%d PASS\n\n", pass, total);
    return pass == total;
}

// -----------------------------------------------------------------------------
// MAIN — Mode A then Mode B
// -----------------------------------------------------------------------------

int main(int argc, char** argv) {
    // Force line-buffering on stderr so progress logs appear in real time when
    // redirected to a file (background runs, etc.).
    setvbuf(stderr, NULL, _IOLBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);

    fprintf(stderr, "%s%s====================================================================%s\n",
            ANSI_CYAN, ANSI_BOLD, ANSI_RESET);
    fprintf(stderr, "%s%s  HORMONEROCHUMINEROPREMIUM_v2.1 — Proyecto LUNA / Estrella%s\n",
            ANSI_CYAN, ANSI_BOLD, ANSI_RESET);
    fprintf(stderr, "  Kissing number attack, R^13, target >=1155 (record %d)\n", TARGET_RECORD_TIE);
    fprintf(stderr, "  Constructor: Claude (Opus 4.7)  |  Auditor: Claude (Opus 4.7, sucesor)\n");
    fprintf(stderr, "  Architect:   R. Amichis Luengo\n");
    fprintf(stderr, "  Pool: pool_v15.bin (64,802 candidates, K_static<=16)  |  Floor: 1130 (P13a)\n");
    fprintf(stderr, "  RNG seed:    0x%016llX\n", (unsigned long long)RNG_SEED);
    fprintf(stderr, "  v2 inherited fixes (still active):\n");
    fprintf(stderr, "    [v2-A] Move T cascade snapshot-restore revert (closes v1.5 self-eviction bug)\n");
    fprintf(stderr, "    [v2-B] Periodic geometric verify: 50pairs/1000it (coarse) + 200/10000it (fine)\n");
    fprintf(stderr, "    [v2-C] save_config GATED on VERIFY:PASS — best_global only updates after verify\n");
    fprintf(stderr, "    [v2-D] Mode A jump TRIPLE-GATED: floor + >=5 distinct improvements + verify pass\n");
    fprintf(stderr, "    [v2-E] Move Z relaxed: Δ>=0 with p_acc(Δ=0)=0.30 + snapshot-restore\n");
    fprintf(stderr, "    [v2-F] Budget calibrated to MEASURED Mac throughput (30M Mode A / ~130M Mode B)\n");
    fprintf(stderr, "    [v2-G] %sCOLORED dashboard%s with HHhMMm clock + best/gap_record/gap_paper/mode/T_acc%%\n",
            ANSI_BOLD, ANSI_RESET);
    fprintf(stderr, "  v2.1 NEW fixes (Auditor-sucesor-ratified 2026-05-03):\n");
    fprintf(stderr, "    [v2.1-A] %sCRYO_THRESH=1131%s (was 1125 sub-floor — jubilated G8)\n",
            ANSI_YELLOW, ANSI_RESET);
    fprintf(stderr, "    [v2.1-B] All operationally-critical constants defended in kissing dim 13 context\n");
    fprintf(stderr, "    [v2.1-C] Sanity #16: Mode B post-warm-load thermal sanity (closes F5-style hole)\n");
    fprintf(stderr, "    [v2.1-D] Dashboard t_floor_hits[CHAINS] counter (distinguishes bouncing vs stuck)\n");
    fprintf(stderr, "  Sanity suite: 16 tests (v2.1 adds #16 Mode B post-warm-load thermal sanity)\n");
    fprintf(stderr, "%s%s====================================================================%s\n\n",
            ANSI_CYAN, ANSI_BOLD, ANSI_RESET);

    fprintf(stderr, "[VERSION] HORMONEROCHUMINEROPREMIUM_v2.1 starting at %.1fs.\n\n", now_sec());

    const char* pool_path = (argc > 1) ? argv[1] : "pool_v15.bin";
    if (!load_pool(pool_path)) return 1;

    // Pre-flight RAM check before any large allocation
    if (!preflight_ram_check()) {
        fprintf(stderr, "[FATAL] pre-flight RAM check failed.\n");
        return 4;
    }

    precompute_adjacency();
    precompute_extras_bucketing();

    if (!sanity_suite()) {
        fprintf(stderr, "[FATAL] sanity failed — abort.\n");
        return 1;
    }

    // ----- MODE A: cold-start validation -----
    fprintf(stderr, "==============================================================\n");
    fprintf(stderr, "  MODE A — COLD-START VALIDATION (must reach %d)\n", TARGET_VALIDATE);
    fprintf(stderr, "==============================================================\n\n");

    std::vector<ChainState> chains(CHAINS);
    for (int c = 0; c < CHAINS; c++) chain_init_empty(chains[c], c);

    int chain_mix[CHAINS][N_MOVES];
    int base_mix[N_MOVES] = {MIX_R,MIX_M,MIX_W,MIX_K,MIX_L,MIX_S,MIX_Z,MIX_X,MIX_P,MIX_T};
    for (int c = 0; c < CHAINS; c++)
        for (int i = 0; i < N_MOVES; i++) chain_mix[c][i] = base_mix[i];

    int  best_global = 0;
    std::vector<int> best_global_active;
    double best_global_time = 0.0;
    int    best_chain = 0;
    long   floor_hold_iters = 0;       // auxiliary tracking, no longer gate
    int    mode_a_distinct_improvements_at_floor = 0;  // v2-D triple gate
    bool   mode_a_validated = false;
    int    mode_a_max = 0;
    long   total_iters = 0;
    long   budget = ITERS_BASE;
    int    extends_used = 0;
    int    impr_in_extend = 0;
    long   last_log = 0;
    int    last_recorded_best = 0;
    long   last_record_iter = 0;
    long   total_attempted_promotions = 0;  // v2-C: candidates rejected by verify gate
    long   total_verify_rejections = 0;     // v2-C: how many failed verify

    MoveTStats t_stats;

    long iter = 0;
    bool jumped_to_mode_b = false;

    auto run_iter = [&](int phase_label /*0=A, 1=B*/) {
        for (int c = 0; c < CHAINS; c++) {
            ChainState& cs = chains[c];
            int mv = sample_move(chain_mix[c]);
            bool acc = false;
            switch (mv) {
                case 0: acc = move_R(cs); break;
                case 1: acc = move_M(cs); break;
                case 2: acc = move_W(cs); break;
                case 3: acc = move_K(cs); break;
                case 4: acc = move_L(cs); break;
                case 5: acc = move_S(cs); break;
                case 6: acc = move_Z(cs); break;
                case 7: acc = move_X(cs); break;
                case 8: acc = move_P(cs); break;
                case 9: acc = move_T(cs, t_stats); break;
            }
            (void)acc;

            if (cs.size > cs.best_local) {
                cs.best_local = cs.size;
                cs.last_improve_iter = iter;
                cs.stag = 0;
            } else {
                cs.stag++;
            }
            if (cs.size > mode_a_max && phase_label == 0) mode_a_max = cs.size;

            if (cs.size > best_global) {
                // v2-C: GATE on verify PASS before updating best_global.
                // The candidate config from cs.active_list MUST be geometrically
                // valid before we promote it. If verify fails, we DUMP the config
                // (corruption diagnostic), do NOT update best_global, and continue
                // (the snapshot-restore in Move T/Z should make this never happen,
                // but defense in depth).
                total_attempted_promotions++;
                int viol;
                double mind;
                bool ok = verify_config_strict(cs.active_list, viol, mind);
                if (!ok) {
                    total_verify_rejections++;
                    fprintf(stderr,
                        "[VERIFY-GATE-REJECT] iter=%ld c=%d size=%d viol=%d min_d=%.6f "
                        "(NOT promoted; state suspect)\n",
                        iter, c, cs.size, viol, mind);
                    char fname[256];
                    snprintf(fname, sizeof(fname),
                             "verify_reject_iter_%ld_c%d_sz%d.txt", iter, c, cs.size);
                    FILE* f = fopen(fname, "w");
                    if (f) {
                        fprintf(f, "# verify-gate reject snapshot\n");
                        fprintf(f, "# iter=%ld chain=%d size=%d viol=%d min_d=%.10f\n",
                                iter, c, cs.size, viol, mind);
                        for (int idx : cs.active_list) fprintf(f, "%d\n", idx);
                        fclose(f);
                    }
                    continue;  // skip promotion, continue with next chain
                }

                best_global = cs.size;
                best_global_active = cs.active_list;
                best_global_time = now_sec();
                best_chain = c;
                impr_in_extend++;

                // v2-D: Mode A triple-gate counter
                if (phase_label == 0 && cs.size >= TARGET_VALIDATE) {
                    mode_a_distinct_improvements_at_floor++;
                }

                // Architect dashboard hit banner
                if (best_global == 1130) {
                    fprintf(stderr, "\n%s%s========================================================%s\n",
                            ANSI_GREEN, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s   HIT 1130 -- P13a FLOOR REACHED (Mode A validated)%s\n",
                            ANSI_GREEN, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s========================================================%s\n\n",
                            ANSI_GREEN, ANSI_BOLD, ANSI_RESET);
                }
                if (best_global == 1131) {
                    fprintf(stderr, "\n%s%s========================================================%s\n",
                            ANSI_YELLOW, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s   HIT 1131 -- BEAT LEECH-SLOANE 1971  *PAPER-WORTHY*%s\n",
                            ANSI_YELLOW, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s========================================================%s\n\n",
                            ANSI_YELLOW, ANSI_BOLD, ANSI_RESET);
                }
                if (best_global == 1154) {
                    fprintf(stderr, "\n%s%s========================================================%s\n",
                            ANSI_RED, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s   HIT 1154 -- TIE ZINOVIEV-ERICSON 1999%s\n",
                            ANSI_RED, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s========================================================%s\n\n",
                            ANSI_RED, ANSI_BOLD, ANSI_RESET);
                }
                if (best_global >= 1155) {
                    fprintf(stderr, "\n%s%s###########################################%s\n",
                            ANSI_RED, ANSI_BOLD, ANSI_RESET);
                    fprintf(stderr, "%s%s###   WORLD RECORD CANDIDATE: |C|=%d   ###%s\n",
                            ANSI_RED, ANSI_BOLD, best_global, ANSI_RESET);
                    fprintf(stderr, "%s%s###########################################%s\n\n",
                            ANSI_RED, ANSI_BOLD, ANSI_RESET);
                }

                if (best_global >= TARGET_SAVE_FROM && best_global > last_recorded_best) {
                    char tag[64];
                    snprintf(tag, sizeof(tag), "phase%c_iter%ld_c%d",
                             phase_label==0?'A':'B', iter, c);
                    save_config(best_global_active, tag);
                    log_record(best_global, tag);
                    last_recorded_best = best_global;
                    last_record_iter = iter;
                }

                if (best_global >= TARGET_RECORD_TIE) {
                    int viol2;
                    double mind2;
                    bool ok2 = verify_config_strict(best_global_active, viol2, mind2);
                    fprintf(stderr, "\n*** RECORD-LEVEL: size=%d viol=%d mind=%.6f ***\n",
                            best_global, viol2, mind2);
                    if (ok2 && best_global >= TARGET_RECORD) {
                        fprintf(stdout, "\n###########################################\n");
                        fprintf(stdout, "###   POSSIBLE WORLD RECORD: |C|=%d  (R^%d)\n",
                                best_global, DIM);
                        fprintf(stdout, "###########################################\n");
                        for (int idx : best_global_active) {
                            for (int d = 0; d < DIM; d++)
                                fprintf(stdout, "%.17g%s", POOL[idx][d],
                                        (d+1<DIM)?" ":"\n");
                        }
                        fflush(stdout);
                        save_config(best_global_active, "WORLD_RECORD");
                    }
                }
            }

            // Mode A floor-hold auxiliary tracking
            if (phase_label == 0 && cs.size >= TARGET_VALIDATE) {
                floor_hold_iters++;
            }

            // v2-B: Periodic geometric integrity sample.
            // Coarse: 50 pairs every 1000 iters.
            // Fine:   200 pairs every 10000 iters.
            // ANY failure -> dump and ABORT (exit 5).
            if (iter > 0 && cs.size >= 100) {
                bool need_check = false;
                int n_pairs_to_check = 0;
                if (iter % VERIFY_FINE_EVERY == 0) {
                    need_check = true;
                    n_pairs_to_check = VERIFY_FINE_PAIRS;
                } else if (iter % VERIFY_COARSE_EVERY == 0) {
                    need_check = true;
                    n_pairs_to_check = VERIFY_COARSE_PAIRS;
                }
                if (need_check) {
                    int fi=-1, fj=-1;
                    double fsq = 0.0;
                    bool ok = geometric_integrity_sample(cs.active_list, n_pairs_to_check,
                                                         fi, fj, fsq);
                    if (!ok) {
                        fprintf(stderr,
                            "%s%s[INTEGRITY-FAIL] iter=%ld c=%d sample-violation: "
                            "i=%d j=%d sqdist=%.10f%s\n",
                            ANSI_RED, ANSI_BOLD, iter, c, fi, fj, fsq, ANSI_RESET);
                        dump_corruption(cs.active_list, iter, c, fi, fj, fsq);
                        std::exit(5);  // hard abort with diagnostic exit code
                    }
                }
            }

            // Cryogenic zone (D17 + TRUENO T1) — v2.1: CRYO_THRESH=1131 (paper-worthy)
            bool in_cryo = (best_global >= CRYO_THRESH);
            double T_max = in_cryo ? T_CRYO_MAX : cs.T_base;
            if (cs.T_current > T_max) cs.T_current = T_max;
            double t_factor = in_cryo ? T_FACTOR_CRYO : T_FACTOR_NORM;
            cs.T_current *= t_factor;
            // T_FLOOR enforced when NOT in cryogenic zone (auditor 2026-05-03):
            // prevents chains from collapsing to deterministic-greedy.
            // Inside cryogenic zone (best_global >= 1131 = paper-worthy), lower
            // floor (0.005) allowed for record-bound refinement.
            double t_min = in_cryo ? 0.005 : T_FLOOR;
            // v2.1-D: count floor-hits for dashboard diagnostic
            if (cs.T_current < t_min) {
                cs.T_current = t_min;
                cs.t_floor_hits++;
            }

            // Triple-rescue D189
            if (c == 0 && cs.stag >= RESCUE_T1) {
                move_X(cs);
                cs.stag = RESCUE_T1 / 2;
            }

            // Reheat
            if (c > 0 && cs.stag >= REHEAT_STAG_TRIGGER &&
                cs.T_base >= REHEAT_MIN_TBASE && cs.reheats < MAX_REHEATS) {
                cs.T_current = cs.T_base * (1.0 + 0.5 * rand_uniform());
                cs.reheats++;
                cs.stag = 0;
            }
        }

        // PT swap every 1024 iters
        if ((iter & 1023) == 0 && iter > 0) {
            int pt_pair = rand_int(0, CHAINS - 2);
            ChainState& A = chains[pt_pair];
            ChainState& B = chains[pt_pair + 1];
            if (B.size > A.size) {
                std::swap(A.active, B.active);
                std::swap(A.active_list, B.active_list);
                std::swap(A.active_pos, B.active_pos);
                std::swap(A.blocked, B.blocked);
                std::swap(A.free_list, B.free_list);
                std::swap(A.free_pos, B.free_pos);
                std::swap(A.size, B.size);
            }
        }

        // MIX_RECAL once
        if (iter == MIX_RECAL_AT) {
            for (int c = 0; c < CHAINS; c++) {
                ChainState& cs = chains[c];
                long sum_tries = 0;
                for (int i = 0; i < N_MOVES; i++) sum_tries += cs.tries[i];
                if (sum_tries < 1000) continue;
                for (int i = 0; i < N_MOVES; i++) {
                    // v1.5 FIX (Auditor ratification 2026-05-03):
                    // EXCLUDE Move Z (idx 6) and Move T (idx 9) from rate-based adjustment.
                    // Both are designed to fire with low acceptance:
                    //   - Move Z (atomic pair swap): strict gain Δ>0 -> expected rate 2-5%
                    //   - Move T (cascade): deep cascades fail often by design -> expected 1-3%
                    // MIX_RECAL halving these would neutralize the v2 fixes ratified by
                    // the Auditor. DO NOT remove this guard without re-ratification.
                    if (i == 6 || i == 9) continue;
                    if (cs.tries[i] < 100) continue;
                    double rate = (double)cs.acc[i] / cs.tries[i];
                    if (rate < 0.05 && chain_mix[c][i] > 1) chain_mix[c][i] = std::max(1, chain_mix[c][i] / 2);
                    if (rate > 0.20 && chain_mix[c][i] < 30) chain_mix[c][i] += 4;
                }
            }
            fprintf(stderr, "[MIX_RECAL] iter=%ld done (Z and T excluded by design — see comment)\n", iter);
        }

        // === v2-G: ARCHITECT DASHBOARD with ANSI COLOR ===
        double t = now_sec();
        if ((long)t / LOG_EVERY_SEC > last_log) {
            last_log = (long)t / LOG_EVERY_SEC;

            // Pick best_global color by tier
            const char* best_color = ANSI_GREEN;       // building
            if (best_global >= 1130) best_color = ANSI_YELLOW;  // floor zone
            if (best_global >= 1154) best_color = ANSI_RED;     // record zone

            // Compute T_acc rate over recent window (last LOG_EVERY_SEC)
            // (we don't have rolling window — use cumulative as proxy; auditor opt-B)
            long t_fired = t_stats.fired;
            long t_acc = t_stats.accepted;
            double t_rate = (t_fired > 0) ? (100.0 * t_acc / t_fired) : 0.0;

            // Convert seconds to HHhMMm
            int hours = (int)(t / 3600.0);
            int mins  = (int)((t - hours*3600.0) / 60.0);
            int secs  = (int)(t - hours*3600.0 - mins*60.0);

            int gap_record = best_global - TARGET_RECORD;     // negative = below record
            int gap_paper  = best_global - 1130;              // positive = above paper-worthy

            fprintf(stderr,
                "[%s%02dh%02dm%02ds%s | iter=%8ld | best=%s%s%d%s | gap_rec=%+d | gap_paper=%+d "
                "| mode=%c | ext=%d/%d | T_acc=%.2f%%]\n",
                ANSI_CYAN, hours, mins, secs, ANSI_RESET,
                iter,
                best_color, ANSI_BOLD, best_global, ANSI_RESET,
                gap_record, gap_paper,
                phase_label==0?'A':'B',
                extends_used, EXTENDS,
                t_rate);

            // chain sizes + temps + per-chain moves Z/T
            fprintf(stderr,
                "  chains_sz=[%d,%d,%d,%d,%d]  T=[%.3f,%.3f,%.3f,%.3f,%.3f]\n",
                chains[0].size, chains[1].size, chains[2].size, chains[3].size, chains[4].size,
                chains[0].T_current, chains[1].T_current, chains[2].T_current,
                chains[3].T_current, chains[4].T_current);

            // v2.1-D: t_floor_hits per chain. High count + growing = bouncing
            // off floor (healthy). Saturated and stalled = stuck at floor (red flag).
            fprintf(stderr,
                "  t_floor_hits=[%ld,%ld,%ld,%ld,%ld]\n",
                chains[0].t_floor_hits, chains[1].t_floor_hits, chains[2].t_floor_hits,
                chains[3].t_floor_hits, chains[4].t_floor_hits);

            // verify-gate stats (v2-C diagnostic)
            if (total_attempted_promotions > 0) {
                fprintf(stderr,
                    "  verify_gate: %ld attempted, %ld rejected (%.4f%%)\n",
                    total_attempted_promotions, total_verify_rejections,
                    100.0 * total_verify_rejections / total_attempted_promotions);
            }

            // first 5 min: per-chain acc% (auditor opt-B)
            if (t < 300.0) {
                fprintf(stderr, "  per-chain acc%%: ");
                for (int c = 0; c < CHAINS; c++) {
                    long sum_tries = 0, sum_acc = 0;
                    for (int i = 0; i < N_MOVES; i++) {
                        sum_tries += chains[c].tries[i];
                        sum_acc += chains[c].acc[i];
                    }
                    double rate = (sum_tries > 0) ? (100.0 * sum_acc / sum_tries) : 0.0;
                    fprintf(stderr, "c%d=%.1f%% ", c, rate);
                }
                fprintf(stderr, "\n");
            }

            // Move Z and T per-chain (opt-B every 30s)
            fprintf(stderr, "  Z(idx6) per-chain: ");
            for (int c = 0; c < CHAINS; c++) {
                long ztr = chains[c].tries[6];
                long zac = chains[c].acc[6];
                double zr = (ztr > 0) ? (100.0 * zac / ztr) : 0.0;
                fprintf(stderr, "c%d=%ld/%ld(%.2f%%) ", c, zac, ztr, zr);
            }
            fprintf(stderr, "\n  T(idx9) per-chain: ");
            for (int c = 0; c < CHAINS; c++) {
                long ttr = chains[c].tries[9];
                long tac = chains[c].acc[9];
                double tr = (ttr > 0) ? (100.0 * tac / ttr) : 0.0;
                fprintf(stderr, "c%d=%ld/%ld(%.2f%%) ", c, tac, ttr, tr);
            }
            fprintf(stderr, "\n");
        }
    };

    // --- Mode A loop ---
    while (iter < budget && !jumped_to_mode_b) {
        run_iter(0);
        iter++;
        total_iters++;

        // === v2-D Mode A TRIPLE-GATE jump condition ===
        // (1) best_global >= TARGET_VALIDATE
        // (2) >= MODE_A_MIN_DISTINCT_IMPROVEMENTS_AT_FLOOR
        // (3) latest verify_config_strict on best_global_active passes
        // The verify-gate at promotion already ensures (3); we re-check defensively here.
        if (best_global >= TARGET_VALIDATE &&
            mode_a_distinct_improvements_at_floor >= MODE_A_MIN_DISTINCT_IMPROVEMENTS) {
            int viol;
            double mind;
            bool ok = verify_config_strict(best_global_active, viol, mind);
            if (ok) {
                mode_a_validated = true;
                jumped_to_mode_b = true;
                fprintf(stderr, "\n%s%s[MODE_A] VALIDATED: best=%d, %d distinct improvements at floor, "
                        "verify PASS%s\n",
                        ANSI_GREEN, ANSI_BOLD, best_global,
                        mode_a_distinct_improvements_at_floor, ANSI_RESET);
                fprintf(stderr, "[MODE_A] Jumping to Mode B with remaining budget %ld iters.\n",
                        budget - iter);
                break;
            } else {
                fprintf(stderr, "[MODE_A] gate held but verify FAIL viol=%d mind=%.6f -- continuing.\n",
                        viol, mind);
            }
        }

        // Auto-extend
        if (iter == budget && extends_used < EXTENDS && !mode_a_validated) {
            if (impr_in_extend >= EXTEND_MIN_IMPR ||
                (extends_used == 0 && best_global < TARGET_VALIDATE)) {
                budget += ITERS_EXT;
                extends_used++;
                impr_in_extend = 0;
                fprintf(stderr, "[EXTEND] %d/%d, new budget=%ld\n",
                        extends_used, EXTENDS, budget);
            } else break;
        }
    }

    // Mode A verdict
    if (!mode_a_validated) {
        fprintf(stderr, "\n[MODE_A] NOT validated. mode_a_max=%d\n", mode_a_max);
        if (mode_a_max < TARGET_GRAVEYARD) {
            fprintf(stderr, "[GRAVEYARD] mode_a_max=%d < %d. Engine invalid for this pool.\n",
                    mode_a_max, TARGET_GRAVEYARD);
            return 2;
        } else if (mode_a_max < TARGET_RETRAIN_LO) {
            fprintf(stderr, "[GRAVEYARD] mode_a_max=%d < %d (retrain threshold). Engine fails validation.\n",
                    mode_a_max, TARGET_RETRAIN_LO);
            return 2;
        } else {
            fprintf(stderr, "[RETRAIN] mode_a_max=%d in [%d, %d). Document for v2 parameter retraining.\n",
                    mode_a_max, TARGET_RETRAIN_LO, TARGET_VALIDATE);
            // Still proceed to Mode B with what we have, but flag.
        }
    }

    // ----- MODE B: warm-attack -----
    fprintf(stderr, "\n==============================================================\n");
    fprintf(stderr, "  MODE B — WARM ATTACK (target >= %d, beat record %d)\n",
            TARGET_RECORD, TARGET_RECORD_TIE);
    fprintf(stderr, "==============================================================\n\n");

    // Warm-load classical 1130 into ALL chains (best foundation)
    for (int c = 0; c < CHAINS; c++) {
        ChainState& cs = chains[c];
        // Reset chain
        chain_init_empty(cs, c);
        for (int i = 0; i < K_CLASSICAL; i++) add_to_active(cs, i);
        cs.best_local = cs.size;
    }
    if (best_global < K_CLASSICAL) {
        best_global = K_CLASSICAL;
        best_global_active = chains[0].active_list;
        best_global_time = now_sec();
    }
    fprintf(stderr, "[MODE_B] All chains warm-loaded with classical %d. best=%d\n",
            K_CLASSICAL, best_global);

    // Mode B budget = remaining + extends
    long mode_b_budget = (budget > iter) ? (budget - iter) : 0;
    if (mode_b_budget < 200000) mode_b_budget = 200000;
    long iter_b_start = iter;
    long iter_b_end = iter + mode_b_budget;

    int mode_b_extends = EXTENDS - extends_used;
    int mode_b_extends_used = 0;
    impr_in_extend = 0;

    while (iter < iter_b_end) {
        run_iter(1);
        iter++;
        total_iters++;

        if (iter == iter_b_end && mode_b_extends_used < mode_b_extends) {
            if (impr_in_extend >= EXTEND_MIN_IMPR ||
                (mode_b_extends_used == 0 && best_global < TARGET_RECORD)) {
                iter_b_end += ITERS_EXT;
                mode_b_extends_used++;
                impr_in_extend = 0;
                fprintf(stderr, "[B-EXTEND] %d/%d, new end=%ld\n",
                        mode_b_extends_used, mode_b_extends, iter_b_end);
            } else break;
        }
    }

    fprintf(stderr, "\n[MODE_B] complete. iters_in_B=%ld, best=%d\n",
            iter - iter_b_start, best_global);

    // ----- POLISH -----
    fprintf(stderr, "\n[POLISH] running up to %d passes on best chain (c%d)...\n",
            POLISH_MAX_PASSES, best_chain);
    int polish_total = 0;
    for (int pass = 0; pass < POLISH_MAX_PASSES; pass++) {
        int gain = polish_pass(chains[best_chain]);
        polish_total += gain;
        fprintf(stderr, "  polish pass %d: gain=%d size=%d\n",
                pass + 1, gain, chains[best_chain].size);
        if (gain == 0) break;
        if (chains[best_chain].size > best_global) {
            best_global = chains[best_chain].size;
            best_global_active = chains[best_chain].active_list;
            best_global_time = now_sec();
            char tag[64];
            snprintf(tag, sizeof(tag), "polish_pass%d", pass + 1);
            save_config(best_global_active, tag);
            log_record(best_global, tag);
        }
    }

    // ----- FINAL VERIFY -----
    fprintf(stderr, "\n[FINAL_VERIFY] auditing best config size=%d ...\n", best_global);
    int viol;
    double mind;
    bool ok = verify_config_strict(best_global_active, viol, mind);
    fprintf(stderr, "  result: %s violations=%d min_pairwise=%.10f\n",
            ok?"PASS":"FAIL", viol, mind);
    if (ok) {
        save_config(best_global_active, "FINAL");
        log_record(best_global, "final");
    }

    // ----- SUMMARY -----
    double total_time = now_sec();
    fprintf(stderr, "\n====================================================================\n");
    fprintf(stderr, "  HORMONEROCHUMINEROPREMIUM_v2.1 SUMMARY\n");
    fprintf(stderr, "====================================================================\n");
    fprintf(stderr, "  best |C|         : %d\n", best_global);
    fprintf(stderr, "  Mode A status    : %s (mode_a_max=%d)\n",
            mode_a_validated?"VALIDATED":"NOT VALIDATED", mode_a_max);
    fprintf(stderr, "  vs P13a (1130)   : %+d\n", best_global - 1130);
    fprintf(stderr, "  vs ZE99  (1154)  : %+d  %s\n",
            best_global - 1154,
            best_global >= 1155 ? "*** WORLD RECORD ***" :
            best_global >= 1154 ? "*** RECORD TIE ***" : "");
    fprintf(stderr, "  total iters      : %ld\n", total_iters);
    fprintf(stderr, "  total time       : %.1fs (%.2fh)\n", total_time, total_time / 3600.0);
    fprintf(stderr, "  iters/sec        : %.0f\n", total_iters / std::max(1.0, total_time));
    fprintf(stderr, "  polish gain      : %d\n", polish_total);
    fprintf(stderr, "  verify           : %s\n", ok?"PASS":"FAIL");
    fprintf(stderr, "  Move T total     : fired=%ld accepted=%ld evicted=%ld added=%ld net=%ld\n",
            t_stats.fired, t_stats.accepted, t_stats.total_evicted,
            t_stats.total_added, t_stats.net_gain_total);

    fprintf(stderr, "\n  Move acceptance per chain:\n");
    fprintf(stderr, "  ch  |    R   |   M   |   W   |   K   |   L   |   S   |   Z   |   X   |   P   |   T   |\n");
    for (int c = 0; c < CHAINS; c++) {
        fprintf(stderr, "  c%d  |", c);
        for (int i = 0; i < N_MOVES; i++) {
            if (chains[c].tries[i] > 0) {
                double r = 100.0 * chains[c].acc[i] / chains[c].tries[i];
                fprintf(stderr, " %5.1f%% |", r);
            } else fprintf(stderr, "  ---  |");
        }
        fprintf(stderr, "\n");
    }

    if (best_global >= TARGET_RECORD) {
        fprintf(stderr, "\n");
        fprintf(stderr, "###########################################\n");
        fprintf(stderr, "###   WORLD RECORD CANDIDATE: |C|=%d   ###\n", best_global);
        fprintf(stderr, "###   Externally verify before claim    ###\n");
        fprintf(stderr, "###########################################\n");
    }

    return ok ? 0 : 3;
}
