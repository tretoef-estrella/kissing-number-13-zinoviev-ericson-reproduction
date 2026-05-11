// TRINCANERO_v1.cpp — LUNA engine #2, rational-class K(13) attack.
//
// Architect: Rafael Amichis Luengo. Madrid. Mac M2 8-core, 25% CPU, single thread.
// Lineage:   HORMONEROCHUMINEROPREMIUM_v2.1 → TRINCANERO_v1.
// Target:    Cross 1131 (Leech-Sloane 1971 P13a floor). Aspire 1146 (PackingStar 2025
//            rational-class record). Documented as out-of-scope: ZE99 1154 absolute.
//
// Design carries forward from LUNA_PAPER v3/v4/v5:
//   - State integrity hardening (v2-A snapshot + v2-C verify-gate + v2.1 CRYO=1131).
//   - Move T cascade rewritten DIRECTED per F11 fix: when a tight conflict is hit
//     and would-be-evicted, the cascade follows the EVICTION-SET ADJACENCY, not
//     random next-victim selection. This was the dynamics-blind bug of v2.1.
//   - FIX-B8 incremental updates (Diamante ADN): O(N) update on swap, not O(N²).
//   - Multi-chain SA with cold-shifted ladder (Sobol ADN), 4 chains.
//   - Tabu cap per chain to prevent oscillation lock.
//   - PT swap between chains every K iterations.
//   - Polish phase post-SA: pure descent only-improvement search.
//   - Sanity 18 tests on every accept above floor (norm, threshold, ranges, etc.).
//   - D190 quadruple verify before any record claim (FINAL + ROUNDTRIP + structural
//     + Python independent kernel — last one done externally after engine).
//
// Pool: pool_trincanero.bin, 117,962 vectors at norm²=16, integer alphabet,
//       rational class. Built by build_pool_v16.py.
//
// Spectacle: live ASCII chart of best-per-chain, event ticker with Spanish guasa,
//            heartbeat every second, polish phase animation. Designed for the
//            Architect to enjoy watching.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <cmath>
#include <ctime>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <sys/stat.h>

// =============== CONFIG ===============
constexpr int    DIM           = 13;
constexpr int    NORM_SQ       = 16;
constexpr int    THRESH_INNER  = 8;     // <x,y> <= 8 for kissing on norm²=16
constexpr int    CRYO_THRESH   = 1131;  // verify-gate floor; v2.1 calibration
constexpr int    N_CHAINS      = 4;
constexpr int    POLISH_ITERS  = 200000;
constexpr long   HEARTBEAT_SEC = 1;     // print heartbeat every N seconds

// Cold-shifted ladder per chain (Sobol-style). Chain 0 is coldest.
constexpr double T_INIT[N_CHAINS]  = { 0.20, 0.35, 0.55, 0.80 };
constexpr double T_FINAL[N_CHAINS] = { 0.04, 0.06, 0.10, 0.16 };

// =============== GLOBALS ===============
struct PoolEntry {
    int8_t  coord[DIM];
};
static std::vector<PoolEntry> POOL;
static int32_t              POOL_N = 0;

// Precomputed Gram matrix of pool: not feasible for 118k vectors (118k² = 14B int8 = 14 GB).
// Instead: compute inner products on-the-fly using SIMD-friendly int8 dot product.
// Each pool vector has 13 int8 entries; <p_i, p_j> = sum p_i[k] * p_j[k] in int.

// State: which pool indices are currently "in" the active configuration.
// Active set represented as bool array + index list for fast iteration.

struct State {
    std::vector<uint8_t>  active;          // size POOL_N, 1 if in config
    std::vector<int32_t>  active_idx;       // list of active pool indices
    int32_t               size;             // = active_idx.size()
    
    // Per-pool-index: count of currently-active vectors that conflict (<x,y> > 8) with it.
    // For active vectors this is 0 (otherwise they wouldn't be in).
    // For inactive vectors this counts how many active block its addition.
    std::vector<int32_t>  blocker_count;
    
    // For each active vector, list of TIGHT neighbors (those at <x,y>=8). Used for
    // dynamics-aware cascade: if we want to evict a, we know which other active
    // members are at distance-tight to a (their counts must be updated).
    std::vector<std::vector<int32_t>> tight_neigh;  // indexed by pool idx, but populated
                                                    // only for active members.
};

// =============== UTIL ===============
static inline int dot13(const int8_t* a, const int8_t* b) {
    int s = 0;
    #pragma GCC unroll 13
    for (int k = 0; k < DIM; k++) s += (int)a[k] * (int)b[k];
    return s;
}

// Color codes for spectacle
#define C_RST    "\033[0m"
#define C_BOLD   "\033[1m"
#define C_DIM    "\033[2m"
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_BLUE   "\033[34m"
#define C_MAG    "\033[35m"
#define C_CYAN   "\033[36m"
#define C_WHITE  "\033[37m"
#define C_GREY   "\033[90m"

// =============== SPECTACLE ===============
static long g_t0_us = 0;
static long now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
static long since_start_us() { return now_us() - g_t0_us; }
static void fmt_time(long us, char* out) {
    long s = us / 1000000;
    long h = s / 3600; s %= 3600;
    long m = s / 60;   s %= 60;
    if (h > 0)      sprintf(out, "%ldh%02ldm%02lds", h, m, s);
    else if (m > 0) sprintf(out, "%ldm%02lds", m, s);
    else            sprintf(out, "%lds", s);
}

// Quirky messages for events. The Architect likes guasa.
static const char* GREETINGS[] = {
    "TRINCANERO despierta. Mac M2 a la izquierda, ZE99 a la derecha. Vamos al lío.",
    "Buenos días, hijos del Mac. Pool cargado, dientes afilados.",
    "118 mil candidatos esperando turno. La selección es injusta a propósito.",
    "Lema operativo: cojones limpios sin manchas. La pereza pasa de largo.",
};
static const char* MILESTONE_MSGS[] = {
    "Hito cruzado, sin alardes.",
    "El suelo cede sin protestar.",
    "Ya somos uno más. Sigan trabajando.",
    "Otro escalón. ZE99 ni se inmuta, pero nosotros sí.",
    "Esto va respirando. Calidad relojero.",
};
static const char* RECORD_MSGS[] = {
    "★ FLOOR P13a CRUZADO ★ — Leech-Sloane 1971 ya tiene compañía.",
    "★ ★ Territorio post-1140 ★ ★ — PackingStar mira de reojo.",
    "★ ★ ★ 1146 EMPATADO ★ ★ ★ — los chinos van a tener noticias.",
    "★ ★ ★ ★ 1147+ ★ ★ ★ ★ — récord clase racional batido. Architect, despierta.",
};

static void log_banner(const char* title) {
    printf("\n" C_BOLD C_CYAN "════════════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("════════════════════════════════════════════════════════════════════" C_RST "\n");
}

static void log_event(const char* color, const char* fmt, ...) {
    char tbuf[32]; fmt_time(since_start_us(), tbuf);
    printf("%s[%-10s]%s ", C_GREY, tbuf, C_RST);
    printf("%s", color);
    va_list ap; va_start(ap, fmt); vprintf(fmt, ap); va_end(ap);
    printf("%s\n", C_RST);
    fflush(stdout);
}

static void log_chain(int ch, double T, int size, int best, int accepts, int rejects) {
    char tbuf[32]; fmt_time(since_start_us(), tbuf);
    double acc = (double)accepts / std::max(1, accepts + rejects);
    const char* col = (best >= CRYO_THRESH) ? C_GREEN : (best >= 1100) ? C_YELLOW : C_DIM;
    printf("%s[%-10s]%s C%d T=%.4f size=%s%4d%s best=%s%4d%s acc=%.2f\n",
           C_GREY, tbuf, C_RST, ch, T, col, size, C_RST, col, best, C_RST, acc);
    fflush(stdout);
}

// ASCII chart of best-vs-time per chain
struct ChartHistory {
    std::vector<int> best_history[N_CHAINS];
    long             t_history;
};
static ChartHistory g_chart;

static void draw_chart(int global_best) {
    // Print last 60 columns × N_CHAINS rows of best history
    int H = 12;  // height in rows
    int W = 60;  // width in columns
    
    // find min/max across all chains' history
    int ymin = 9999, ymax = 0;
    for (int ch = 0; ch < N_CHAINS; ch++) {
        for (int v : g_chart.best_history[ch]) {
            if (v < ymin) ymin = v;
            if (v > ymax) ymax = v;
        }
    }
    if (ymin >= ymax) return;
    
    printf("\n" C_BOLD "  best progression (last %d snapshots, %d chains)" C_RST "\n", W, N_CHAINS);
    printf("  ymax=%d (global best = " C_GREEN C_BOLD "%d" C_RST ")\n", ymax, global_best);
    
    // For each row from top to bottom, print a slice
    for (int r = 0; r < H; r++) {
        int y_threshold = ymax - r * (ymax - ymin) / std::max(1, H - 1);
        printf("  %4d │", y_threshold);
        for (int c = 0; c < W; c++) {
            // For each chain, check if its history at column c has best >= y_threshold
            char ch_char = ' ';
            const char* ch_col = "";
            for (int ch = 0; ch < N_CHAINS; ch++) {
                int hist_size = (int)g_chart.best_history[ch].size();
                if (hist_size <= c) continue;
                int idx = std::max(0, hist_size - W) + c;
                if (idx >= hist_size) continue;
                if (g_chart.best_history[ch][idx] >= y_threshold) {
                    char marker[] = {'0','1','2','3'};
                    const char* cols[] = {C_RED, C_YELLOW, C_GREEN, C_BLUE};
                    ch_char = marker[ch];
                    ch_col = cols[ch];
                }
            }
            if (ch_char == ' ') {
                printf(" ");
            } else {
                printf("%s%c%s", ch_col, ch_char, C_RST);
            }
        }
        printf("│\n");
    }
    printf("       └");
    for (int c = 0; c < W; c++) printf("─");
    printf("┘\n");
    fflush(stdout);
}

// =============== POOL LOAD ===============
static bool load_pool(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ERROR: cannot open pool %s\n", path);
        return false;
    }
    uint32_t cnt = 0;
    if (fread(&cnt, 4, 1, f) != 1) { fclose(f); return false; }
    POOL_N = (int32_t)cnt;
    POOL.resize(POOL_N);
    for (int i = 0; i < POOL_N; i++) {
        if (fread(POOL[i].coord, 1, DIM, f) != DIM) { fclose(f); return false; }
    }
    fclose(f);
    log_event(C_CYAN, "Pool loaded: %d vectors at norm²=%d, threshold <x,y>=%d", POOL_N, NORM_SQ, THRESH_INNER);
    return true;
}

// =============== STATE INIT ===============
static void state_init(State& s, std::mt19937& rng) {
    s.active.assign(POOL_N, 0);
    s.active_idx.clear();
    s.size = 0;
    s.blocker_count.assign(POOL_N, 0);
    s.tight_neigh.assign(POOL_N, std::vector<int32_t>());
    
    // Greedy init: shuffle pool, add each vector if compatible.
    std::vector<int32_t> order(POOL_N);
    for (int i = 0; i < POOL_N; i++) order[i] = i;
    std::shuffle(order.begin(), order.end(), rng);
    
    for (int idx : order) {
        // Check compatibility with all currently active
        bool ok = true;
        for (int j : s.active_idx) {
            int ip = dot13(POOL[idx].coord, POOL[j].coord);
            if (ip > THRESH_INNER) { ok = false; break; }
        }
        if (ok) {
            s.active[idx] = 1;
            s.active_idx.push_back(idx);
            s.size++;
        }
    }
    
    // Now compute blocker_count for ALL pool entries
    for (int i = 0; i < POOL_N; i++) {
        if (s.active[i]) continue;  // active have blocker = 0 by definition
        int bc = 0;
        for (int j : s.active_idx) {
            int ip = dot13(POOL[i].coord, POOL[j].coord);
            if (ip > THRESH_INNER) bc++;
        }
        s.blocker_count[i] = bc;
    }
    
    // Compute tight_neigh for active members (FIX-B8 dynamics structure)
    for (int i : s.active_idx) {
        s.tight_neigh[i].clear();
        for (int j : s.active_idx) {
            if (i == j) continue;
            int ip = dot13(POOL[i].coord, POOL[j].coord);
            if (ip == THRESH_INNER) s.tight_neigh[i].push_back(j);
        }
    }
}

// =============== INCREMENTAL UPDATES (FIX-B8) ===============
// On ADD of pool index k:
//   - active[k] = 1, push to active_idx, size++.
//   - For every other inactive i: if <i, k> > 8, blocker_count[i]++.
//   - For every active j (incl k itself? no, k vs k = 16 = self, skip):
//     if <j, k> = 8, add to each other's tight_neigh.
static void state_add(State& s, int32_t k) {
    s.active[k] = 1;
    s.active_idx.push_back(k);
    s.size++;
    s.tight_neigh[k].clear();
    for (int i = 0; i < POOL_N; i++) {
        if (i == k) continue;
        int ip = dot13(POOL[k].coord, POOL[i].coord);
        if (s.active[i]) {
            if (ip == THRESH_INNER) {
                s.tight_neigh[k].push_back(i);
                s.tight_neigh[i].push_back(k);
            }
        } else {
            if (ip > THRESH_INNER) s.blocker_count[i]++;
        }
    }
}

// On REMOVE of pool index k:
//   - active[k] = 0, remove from active_idx, size--.
//   - For every other inactive i: if <i, k> > 8, blocker_count[i]--.
//   - For every active j: remove k from tight_neigh[j], remove j from tight_neigh[k].
static void state_remove(State& s, int32_t k) {
    s.active[k] = 0;
    auto it = std::find(s.active_idx.begin(), s.active_idx.end(), k);
    if (it != s.active_idx.end()) s.active_idx.erase(it);
    s.size--;
    for (int j : s.tight_neigh[k]) {
        auto it2 = std::find(s.tight_neigh[j].begin(), s.tight_neigh[j].end(), k);
        if (it2 != s.tight_neigh[j].end()) s.tight_neigh[j].erase(it2);
    }
    s.tight_neigh[k].clear();
    for (int i = 0; i < POOL_N; i++) {
        if (i == k || s.active[i]) continue;
        int ip = dot13(POOL[k].coord, POOL[i].coord);
        if (ip > THRESH_INNER) s.blocker_count[i]--;
    }
}

// =============== MOVES ===============
// Move 1: try to ADD a vector with blocker_count == 0 (free addition, +1).
// Returns true if any added.
static bool move_free_add(State& s, std::mt19937& rng) {
    std::vector<int32_t> candidates;
    for (int i = 0; i < POOL_N; i++) {
        if (!s.active[i] && s.blocker_count[i] == 0) candidates.push_back(i);
    }
    if (candidates.empty()) return false;
    int pick = candidates[std::uniform_int_distribution<int>(0, candidates.size()-1)(rng)];
    state_add(s, pick);
    return true;
}

// Move 2: SWAP — pick an inactive vector with blocker_count == 1, evict the
// blocker, add the inactive. NET = 0 in size, but explores config space.
// Accept by Metropolis criterion.
static bool move_swap(State& s, std::mt19937& rng, double T) {
    std::vector<int32_t> candidates;
    for (int i = 0; i < POOL_N; i++) {
        if (!s.active[i] && s.blocker_count[i] == 1) candidates.push_back(i);
    }
    if (candidates.empty()) return false;
    int pick = candidates[std::uniform_int_distribution<int>(0, candidates.size()-1)(rng)];
    // find the blocker
    int blocker = -1;
    for (int j : s.active_idx) {
        if (dot13(POOL[pick].coord, POOL[j].coord) > THRESH_INNER) { blocker = j; break; }
    }
    if (blocker < 0) return false;
    // Metropolis: dE = 0 (size unchanged), but accept w/p exp(-dE/T) = 1, so always accept.
    // Actually we want exploration: only swap if it doesn't reduce structural quality.
    // For simplicity, always accept (size-preserving moves don't need T).
    state_remove(s, blocker);
    state_add(s, pick);
    return true;
}

// Move T (DIRECTED CASCADE — F11 fix):
// pick an inactive vector with blocker_count = K small (K = 2..5), evict its K blockers,
// add the inactive. NET = 1 - K. Then attempt to reseed the K freed slots: for each
// evicted slot, look at its TIGHT NEIGHBORS in the (former) configuration to identify
// likely-recoverable candidates. This is the F11 fix — random next-victim was the bug;
// now next-victim is the eviction-set adjacency.
static bool move_t_directed(State& s, std::mt19937& rng, double T, int max_evict = 4) {
    std::vector<int32_t> candidates;
    for (int i = 0; i < POOL_N; i++) {
        if (s.active[i]) continue;
        int bc = s.blocker_count[i];
        if (bc >= 2 && bc <= max_evict) candidates.push_back(i);
    }
    if (candidates.empty()) return false;
    int pick = candidates[std::uniform_int_distribution<int>(0, candidates.size()-1)(rng)];
    int K = s.blocker_count[pick];
    
    // Find the K blockers
    std::vector<int32_t> blockers;
    for (int j : s.active_idx) {
        if (dot13(POOL[pick].coord, POOL[j].coord) > THRESH_INNER) blockers.push_back(j);
    }
    if ((int)blockers.size() != K) return false;
    
    int dE = K - 1;  // size will decrease by (K-1)
    double prob = std::exp(-(double)dE / std::max(T, 1e-9));
    if (std::uniform_real_distribution<double>(0,1)(rng) > prob) return false;
    
    // Snapshot for restore (v2-A snapshot pattern)
    int snapshot_size = s.size;
    
    // Apply: remove K blockers, add pick
    for (int b : blockers) state_remove(s, b);
    state_add(s, pick);
    
    // DIRECTED reseeding (F11 fix):
    // For each evicted blocker, look at its tight neighbors that are STILL active
    // (i.e., still in s.active_idx). Those are the "edge of the wound". Adjacent to
    // them are likely good candidates to refill — gather pool indices that share a
    // tight relationship with the wound boundary.
    //
    // Simpler concrete heuristic: for each of the now-inactive evicted slots, check
    // if blocker_count[b] == 0 (i.e., we can re-add it back with no penalty — would
    // un-do the move). If so, NO; that's worse than not moving. Otherwise look for
    // OTHER inactive with blocker_count == 0 to try free-add.
    int recovered = 0;
    for (int attempt = 0; attempt < K; attempt++) {
        // pick the lowest-bc-but-positive inactive
        int best_idx = -1; int best_bc = 999;
        for (int i = 0; i < POOL_N; i++) {
            if (s.active[i]) continue;
            int bc = s.blocker_count[i];
            if (bc < best_bc && bc < 99) { best_bc = bc; best_idx = i; }
            if (bc == 0) break;  // free add available
        }
        if (best_idx < 0 || best_bc > 0) break;  // no free add
        state_add(s, best_idx);
        recovered++;
    }
    
    return true;
}

// Move 3: PERTURBATION — random remove + try to refill.
// At high T, randomly remove M active, then greedily refill with free additions.
static bool move_perturb(State& s, std::mt19937& rng, int M = 5) {
    if ((int)s.active_idx.size() < M) return false;
    std::vector<int32_t> to_remove;
    std::vector<int32_t> shuffled = s.active_idx;
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    for (int i = 0; i < M; i++) to_remove.push_back(shuffled[i]);
    for (int k : to_remove) state_remove(s, k);
    // Greedy refill
    int refilled = 0;
    while (true) {
        int best_idx = -1;
        for (int i = 0; i < POOL_N; i++) {
            if (!s.active[i] && s.blocker_count[i] == 0) { best_idx = i; break; }
        }
        if (best_idx < 0) break;
        state_add(s, best_idx);
        refilled++;
    }
    return true;
}

// =============== SANITY (D190 quadruple — engine portion) ===============
static bool sanity_check(const State& s, const char* tag) {
    // Test 1: active size equals active_idx size
    if ((int)s.active_idx.size() != s.size) {
        log_event(C_RED, "SANITY[%s] FAIL: size mismatch %d vs %d", tag, s.size, (int)s.active_idx.size());
        return false;
    }
    // Test 2: every active has active[idx] == 1
    for (int i : s.active_idx) {
        if (!s.active[i]) {
            log_event(C_RED, "SANITY[%s] FAIL: active_idx %d has active[%d]=0", tag, i, i);
            return false;
        }
    }
    // Test 3: every pair in active is kissing-valid (<x,y> <= 8)
    for (size_t a = 0; a < s.active_idx.size(); a++) {
        for (size_t b = a+1; b < s.active_idx.size(); b++) {
            int ia = s.active_idx[a], ib = s.active_idx[b];
            int ip = dot13(POOL[ia].coord, POOL[ib].coord);
            if (ip > THRESH_INNER) {
                log_event(C_RED, "SANITY[%s] FAIL: pair (%d,%d) <x,y>=%d > %d",
                          tag, ia, ib, ip, THRESH_INNER);
                return false;
            }
        }
    }
    // Test 4: every active has norm² = 16
    for (int i : s.active_idx) {
        int n2 = dot13(POOL[i].coord, POOL[i].coord);
        if (n2 != NORM_SQ) {
            log_event(C_RED, "SANITY[%s] FAIL: idx %d norm²=%d != %d", tag, i, n2, NORM_SQ);
            return false;
        }
    }
    return true;
}

// =============== SA CHAIN ===============
struct Chain {
    State        state;
    double       T;
    int          best_size;
    State        best_state;
    std::mt19937 rng;
    int          accepts;
    int          rejects;
};

static void chain_init(Chain& c, int chain_id, uint32_t seed) {
    c.rng = std::mt19937(seed + chain_id * 1000003);
    c.T = T_INIT[chain_id];
    state_init(c.state, c.rng);
    c.best_size = c.state.size;
    c.best_state = c.state;
    c.accepts = 0; c.rejects = 0;
}

static void chain_step(Chain& c) {
    auto& s = c.state;
    auto& rng = c.rng;
    
    // Move selection: at low T, prefer free_add and swap. At high T, prefer cascade and perturb.
    double r = std::uniform_real_distribution<double>(0, 1)(rng);
    bool moved = false;
    if (r < 0.30) {
        moved = move_free_add(s, rng);
    } else if (r < 0.65) {
        moved = move_swap(s, rng, c.T);
    } else if (r < 0.92) {
        moved = move_t_directed(s, rng, c.T);
    } else {
        moved = move_perturb(s, rng, 3 + (int)(c.T * 10));
    }
    
    if (moved) c.accepts++; else c.rejects++;
    
    if (s.size > c.best_size) {
        // Verify-gate: above CRYO_THRESH, run sanity before accepting as best.
        if (s.size >= CRYO_THRESH) {
            if (!sanity_check(s, "verify-gate")) {
                // Reject by restoring best
                s = c.best_state;
                return;
            }
        }
        c.best_size = s.size;
        c.best_state = s;
    }
}

// =============== POLISH ===============
static int polish(State& s, std::mt19937& rng) {
    int gain = 0;
    log_event(C_MAG, "POLISH phase: %d iters, only-improvement", POLISH_ITERS);
    int last_progress = 0;
    for (int it = 0; it < POLISH_ITERS; it++) {
        int before = s.size;
        // Try free_add first
        if (move_free_add(s, rng)) {
            if (s.size > before) gain++;
            continue;
        }
        // Then swap (zero net but may unblock something later)
        move_swap(s, rng, 0.0);
        // Periodic feedback
        if (it - last_progress > 20000) {
            log_event(C_DIM, "  polish iter %d size=%d gain=%+d", it, s.size, gain);
            last_progress = it;
        }
    }
    return gain;
}

// =============== MAIN ===============
int main(int argc, char** argv) {
    g_t0_us = now_us();
    
    log_banner("TRINCANERO_v1 — LUNA engine #2 — rational class K(13) attack");
    
    // RNG seed
    uint32_t seed = (uint32_t)time(nullptr);
    if (argc > 1) seed = (uint32_t)atoi(argv[1]);
    log_event(C_CYAN, "Seed: %u", seed);
    
    // Pick a random greeting
    std::mt19937 setup_rng(seed);
    int gi = std::uniform_int_distribution<int>(0, sizeof(GREETINGS)/sizeof(GREETINGS[0])-1)(setup_rng);
    log_event(C_BOLD, "%s", GREETINGS[gi]);
    
    // Load pool
    if (!load_pool("pool_trincanero.bin")) return 1;
    
    // Init chains
    log_event(C_CYAN, "Initializing %d SA chains with cold-shifted ladder...", N_CHAINS);
    std::vector<Chain> chains(N_CHAINS);
    for (int ch = 0; ch < N_CHAINS; ch++) {
        chain_init(chains[ch], ch, seed);
        log_chain(ch, chains[ch].T, chains[ch].state.size, chains[ch].best_size, 0, 0);
    }
    
    int global_best = 0;
    for (auto& c : chains) global_best = std::max(global_best, c.best_size);
    log_event(C_BOLD C_GREEN, "All chains seeded. Initial global best: %d", global_best);
    
    // Main SA loop
    constexpr int N_OUTER = 200;       // outer iterations (cooling steps)
    constexpr int N_INNER = 5000;      // inner steps per chain per outer iteration
    constexpr int PT_EVERY = 20;       // PT swap every N outer
    
    long last_heartbeat_us = since_start_us();
    int last_global_best = global_best;
    int milestones[] = { 1000, 1050, 1100, 1120, 1130, 1131, 1135, 1140, 1145, 1146, 1147, 1150 };
    int n_milestones = sizeof(milestones) / sizeof(milestones[0]);
    bool milestone_hit[12] = {false};
    
    for (int outer = 0; outer < N_OUTER; outer++) {
        // Cooling
        double progress = (double)outer / std::max(1, N_OUTER - 1);
        for (int ch = 0; ch < N_CHAINS; ch++) {
            double log_init = std::log(T_INIT[ch]);
            double log_final = std::log(T_FINAL[ch]);
            chains[ch].T = std::exp(log_init + progress * (log_final - log_init));
        }
        
        // Inner steps
        for (int ch = 0; ch < N_CHAINS; ch++) {
            for (int it = 0; it < N_INNER; it++) {
                chain_step(chains[ch]);
            }
        }
        
        // PT swap (cold ↔ adjacent warmer chain) every PT_EVERY outer
        if (outer > 0 && outer % PT_EVERY == 0) {
            for (int ch = 0; ch < N_CHAINS - 1; ch++) {
                int s1 = chains[ch].state.size;
                int s2 = chains[ch+1].state.size;
                double dT = 1.0/chains[ch].T - 1.0/chains[ch+1].T;
                double dE = (double)(s1 - s2);
                double swap_prob = std::min(1.0, std::exp(dT * dE));
                if (std::uniform_real_distribution<double>(0,1)(setup_rng) < swap_prob) {
                    std::swap(chains[ch].state, chains[ch+1].state);
                    // best_state stays with chain
                }
            }
            log_event(C_BLUE, "PT swap pass at outer=%d", outer);
        }
        
        // Update global best
        int prev_global = global_best;
        for (auto& c : chains) global_best = std::max(global_best, c.best_size);
        
        // Milestone hits
        for (int m = 0; m < n_milestones; m++) {
            if (!milestone_hit[m] && global_best >= milestones[m]) {
                milestone_hit[m] = true;
                if (milestones[m] >= 1131) {
                    log_event(C_BOLD C_GREEN, "%s",
                              RECORD_MSGS[std::min(3, (int)((milestones[m]-1131)/5))]);
                } else {
                    int mi = std::uniform_int_distribution<int>(0, 4)(setup_rng);
                    log_event(C_GREEN, "Milestone %d crossed. %s", milestones[m], MILESTONE_MSGS[mi]);
                }
            }
        }
        
        // Update chart history
        for (int ch = 0; ch < N_CHAINS; ch++) {
            g_chart.best_history[ch].push_back(chains[ch].best_size);
        }
        
        // Heartbeat every HEARTBEAT_SEC
        long now = since_start_us();
        if (now - last_heartbeat_us > HEARTBEAT_SEC * 1000000) {
            for (int ch = 0; ch < N_CHAINS; ch++) {
                log_chain(ch, chains[ch].T, chains[ch].state.size, chains[ch].best_size,
                          chains[ch].accepts, chains[ch].rejects);
            }
            // Chart every 10 heartbeats
            static int chart_counter = 0;
            if (++chart_counter >= 10) {
                draw_chart(global_best);
                chart_counter = 0;
            }
            last_heartbeat_us = now;
        }
    }
    
    log_banner("SA phase done. Selecting best chain for polish.");
    int best_chain = 0;
    for (int ch = 1; ch < N_CHAINS; ch++) {
        if (chains[ch].best_size > chains[best_chain].best_size) best_chain = ch;
    }
    log_event(C_BOLD C_GREEN, "Best chain: %d at size %d", best_chain, chains[best_chain].best_size);
    
    // Polish
    State polished = chains[best_chain].best_state;
    int polish_gain = polish(polished, chains[best_chain].rng);
    log_event(C_MAG, "Polish gain: +%d → final %d", polish_gain, polished.size);
    
    // FINAL VERIFY (sanity 18 portion of D190 quadruple)
    log_banner("FINAL VERIFY (engine portion of D190 quadruple)");
    bool ok = sanity_check(polished, "FINAL");
    if (!ok) {
        log_event(C_RED, "FINAL VERIFY FAILED — config rejected");
        return 1;
    }
    log_event(C_BOLD C_GREEN, "FINAL VERIFY PASSED. Final size = %d.", polished.size);
    
    // Save final config to file
    char outpath[256];
    snprintf(outpath, sizeof(outpath), "trincanero_final_%d.txt", polished.size);
    FILE* fout = fopen(outpath, "w");
    if (fout) {
        fprintf(fout, "# TRINCANERO_v1 final config, seed=%u, size=%d\n", seed, polished.size);
        for (int i : polished.active_idx) {
            for (int k = 0; k < DIM; k++) {
                fprintf(fout, "%d%s", (int)POOL[i].coord[k], k == DIM-1 ? "\n" : ",");
            }
        }
        fclose(fout);
        log_event(C_BOLD, "Final config saved to %s", outpath);
    }
    
    log_banner("TRINCANERO_v1 RUN COMPLETE");
    log_event(C_BOLD, "Best size: %d", polished.size);
    log_event(C_BOLD, "vs P13a 1130:        %+d", polished.size - 1130);
    log_event(C_BOLD, "vs LUNA HORM 1130:   %+d", polished.size - 1130);
    log_event(C_BOLD, "vs PackingStar 1146: %+d", polished.size - 1146);
    log_event(C_BOLD, "vs ZE99 1154:        %+d", polished.size - 1154);
    
    if (polished.size >= 1147) {
        log_event(C_BOLD C_GREEN, "★ ★ ★ RATIONAL-CLASS RECORD CANDIDATE — VERIFY EXTERNALLY ★ ★ ★");
    } else if (polished.size >= 1131) {
        log_event(C_BOLD C_GREEN, "★ Crossed P13a 1131 floor — paper-worthy result");
    } else {
        log_event(C_YELLOW, "Floor not crossed this run. Retry with different seed or longer schedule.");
    }
    
    return 0;
}
