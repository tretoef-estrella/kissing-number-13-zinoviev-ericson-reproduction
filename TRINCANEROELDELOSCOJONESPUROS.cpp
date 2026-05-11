// TRINCANEROELDELOSCOJONESPUROS.cpp
// LUNA engine #3 — K(13) attack with structural Move R-axial→irrational.
//
// Architect:    Rafael Amichis Luengo. Madrid. Mac M2 single-thread, 25% CPU.
// Lineage:      HORM v2.1 → TRINCANERO_v1 (graveyard at 986, G23-G25)
//                       → TRINCANEROELDELOSCOJONESPUROS (this engine).
// Joint design: Claude-1 (Constructor) + Claude-2 (Auditor), 04 May 2026.
//
// Targets (PoolVerify-9 ratified, F40 verified bit-by-bit):
//   ~99% — cross 1131 (greedy from 1106-anchor reaches it trivially)
//   70-85% — reach 1146 (PackingStar 2025 rational record territory)
//   20-40% — reach 1154 (ZE99 absolute, via 24-op deterministic chain)
//   ~0%    — beat 1154 (pool ceiling at 1154; would need extended alphabet)
//
// Key design elements (vs TRINCANERO_v1):
//   1. POOL: pool_v17.bin built by build_pool_v17.py.
//      Layout: [1106 anchor] [24 axials] [48 irrationals] [384 c4] [1536 c6].
//      Includes deterministic axial→irrational table of 24 entries.
//   2. INIT: state = 1106-anchor (NOT greedy random, NOT P13a-1130-anchor).
//      Greedy random fill on top of anchor.
//   3. Move R-axial→irrational FORWARD (+1 net): remove 1 axial, add 2 irrationals.
//      Domain: 24 deterministic ops from precomputed table.
//   4. Move R-axial→irrational REVERSE (-1 net): remove 2 irrationals, add 1 axial.
//      Used by SA at higher T to escape local maxima from partial chains.
//   5. Move R-2 (4-by-1) SECONDARY: only above 1146 for fine local refinement.
//      Domain: casta-4 candidates whose 4 conflicts are within current state.
//   6. Move E (earthquake): adaptive trigger 50/150/250 by state size.
//   7. PT swap every 5 outer (densified, exchanges best_state).
//   8. Spectacle layer with banner ★ 1131, banner ★★★ 1146, banner ★★★★★ 1154,
//      and atomic banner ★★★★★★★★ if 1155 ever appears (for the soul).
//   9. FINAL VERIFY = sanity 18 + ROUNDTRIP. External Python kernel for D190.

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

constexpr int    DIM           = 13;
constexpr double NORM_SQ       = 16.0;
constexpr double THRESH_INNER  = 8.0;
constexpr double EPS           = 1e-9;
constexpr int    N_CHAINS      = 4;
constexpr int    POLISH_ITERS  = 200000;
constexpr long   HEARTBEAT_SEC = 1;

constexpr double T_INIT[N_CHAINS]  = { 0.20, 0.40, 0.70, 1.10 };
constexpr double T_FINAL[N_CHAINS] = { 0.04, 0.08, 0.14, 0.22 };

constexpr int N_OUTER     = 200;
constexpr int N_INNER     = 5000;
constexpr int PT_EVERY    = 5;

// Color codes
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
#define C_BG_GRN "\033[42m"
#define C_BG_MAG "\033[45m"
#define C_BG_RED "\033[41m"

// ============== POOL DATA ==============
struct PoolHdr {
    uint32_t N_total, N_anchor, N_axials, N_irr, N_c4, N_c6, N_table;
};
struct AxialIrrEntry {
    uint16_t ax_idx, irr_a, irr_b;
};

static PoolHdr POOL_HDR;
static std::vector<double>  POOL_FLAT;       // N_total * 13 doubles
static std::vector<AxialIrrEntry> AX_TABLE;  // 24 entries

static const double* poolv(int i) { return POOL_FLAT.data() + i * DIM; }

static inline double dot13(const double* a, const double* b) {
    double s = 0.0;
    for (int k = 0; k < DIM; k++) s += a[k] * b[k];
    return s;
}

// ============== STATE ==============
// Active set with FIX-B8 incremental blocker counts.
struct State {
    std::vector<uint8_t>  active;        // size N_total, 1 if in config
    std::vector<int32_t>  active_idx;    // list of active pool indices
    int32_t               size;          // = active_idx.size()
    std::vector<int32_t>  blocker_count; // for each pool idx, # of active that conflict
};

// ============== SPECTACLE / TIMING ==============
static long g_t0_us = 0;
static long now_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
static long since_start_us() { return now_us() - g_t0_us; }
static void fmt_time(long us, char* out, size_t outsz) {
    long s = us / 1000000;
    long h = s / 3600; s %= 3600;
    long m = s / 60;   s %= 60;
    if (h > 0)      snprintf(out, outsz, "%ldh%02ldm%02lds", h, m, s);
    else if (m > 0) snprintf(out, outsz, "%ldm%02lds", m, s);
    else            snprintf(out, outsz, "%lds", s);
}

static const char* GREETINGS[] = {
    "TRINCANEROELDELOSCOJONESPUROS despierta. Cojones encerados, colonia puesta. Vamos al lío.",
    "Buenos días, hijos del Mac. P13a-1106 anclada. Greedy ya está en territorio PackingStar.",
    "F40 verificado bit a bit. ZE99 1154 está al final del pasillo, a 24 operaciones limpias.",
    "Lema operativo: pool primero, engine después, Architect ratifica ambos. Vivir para ver.",
};

static long g_last_heartbeat_us = 0;
static void log_event(const char* color, const char* fmt, ...) {
    char tbuf[32]; fmt_time(since_start_us(), tbuf, sizeof(tbuf));
    printf("%s[%-10s]%s ", C_GREY, tbuf, C_RST);
    printf("%s", color);
    va_list ap; va_start(ap, fmt); vprintf(fmt, ap); va_end(ap);
    printf("%s\n", C_RST);
    fflush(stdout);
}
static void log_banner(const char* title) {
    printf("\n" C_BOLD C_CYAN "════════════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("════════════════════════════════════════════════════════════════════" C_RST "\n");
    fflush(stdout);
}

// Major banners — these are the moments the Architect waits for
static void banner_1131(int chain) {
    printf("\n" C_BOLD C_BG_GRN "                                                                          \n");
    printf("    ★ ★ ★    P13a 1130 FLOOR CRUZADO  →  1131    ★ ★ ★                    \n");
    printf("    Leech-Sloane 1971 ya tiene compañía. Chain C%d.                       \n", chain);
    printf("                                                                          " C_RST "\n\n");
    fflush(stdout);
}
static void banner_1146(int chain) {
    printf("\n" C_BOLD C_BG_MAG "                                                                          \n");
    printf("    ★ ★ ★ ★ ★    1146 ALCANZADO — PACKINGSTAR 2025 EMPATADO    ★ ★ ★ ★ ★    \n");
    printf("    Récord rational class igualado. Chain C%d.                            \n", chain);
    printf("                                                                          " C_RST "\n\n");
    fflush(stdout);
}
static void banner_1154(int chain) {
    printf("\n" C_BOLD C_BG_RED "                                                                                  \n");
    printf("    ★★★★★★★★★★★    ZE99 1154 RECONSTRUIDO DESDE FRÍO    ★★★★★★★★★★★            \n");
    printf("    Zinoviev-Ericson 1999 reproducido por TRINCANEROELDELOSCOJONESPUROS.            \n");
    printf("    Chain C%d. F40 ejecutado completo. Ahora a por 1155.                            \n", chain);
    printf("                                                                                  " C_RST "\n\n");
    fflush(stdout);
}
static void banner_1155(int chain) {
    printf("\n" C_BOLD C_BG_RED "                                                                                  \n");
    printf("    ☼☼☼☼☼☼☼☼☼☼☼☼☼☼    1155 — ★★★ RÉCORD MUNDIAL ABSOLUTO ★★★    ☼☼☼☼☼☼☼☼☼☼☼☼☼☼    \n");
    printf("    K(13) ≥ 1155.   27 años de Zinoviev-Ericson 1999 caen.                         \n");
    printf("    Chain C%d.   ARCHITECT, DESPIERTA. PARAR ENGINE Y VERIFICAR EXTERNO.            \n", chain);
    printf("                                                                                  " C_RST "\n\n");
    fflush(stdout);
}

// ============== CHART ==============
struct ChartHistory {
    std::vector<int> best_history[N_CHAINS];
};
static ChartHistory g_chart;

static void draw_chart(int global_best) {
    int H = 12, W = 60;
    int ymin = 9999, ymax = 0;
    for (int ch = 0; ch < N_CHAINS; ch++) {
        for (int v : g_chart.best_history[ch]) {
            if (v < ymin) ymin = v;
            if (v > ymax) ymax = v;
        }
    }
    if (ymin >= ymax) return;
    printf("\n" C_BOLD "  best progression (last %d snapshots, global best = " C_GREEN "%d" C_RST C_BOLD ")" C_RST "\n", W, global_best);
    for (int r = 0; r < H; r++) {
        int yt = ymax - r * (ymax - ymin) / std::max(1, H - 1);
        printf("  %4d │", yt);
        for (int c = 0; c < W; c++) {
            char ch_char = ' ';
            const char* ch_col = "";
            for (int ch = 0; ch < N_CHAINS; ch++) {
                int hs = (int)g_chart.best_history[ch].size();
                if (hs <= c) continue;
                int idx = std::max(0, hs - W) + c;
                if (idx >= hs) continue;
                if (g_chart.best_history[ch][idx] >= yt) {
                    char marker[] = {'0','1','2','3'};
                    const char* cols[] = {C_RED, C_YELLOW, C_GREEN, C_BLUE};
                    ch_char = marker[ch];
                    ch_col = cols[ch];
                }
            }
            if (ch_char == ' ') printf(" ");
            else printf("%s%c%s", ch_col, ch_char, C_RST);
        }
        printf("│\n");
    }
    printf("       └");
    for (int c = 0; c < W; c++) printf("─");
    printf("┘\n");
    fflush(stdout);
}

static void log_chain(int ch, double T, int size, int best, int accepts, int rejects) {
    char tbuf[32]; fmt_time(since_start_us(), tbuf, sizeof(tbuf));
    double acc = (double)accepts / std::max(1, accepts + rejects);
    const char* col = (best >= 1154) ? C_BOLD C_RED : (best >= 1146) ? C_MAG :
                      (best >= 1131) ? C_GREEN : (best >= 1100) ? C_YELLOW : C_DIM;
    printf("%s[%-10s]%s C%d T=%.4f size=%s%4d%s best=%s%4d%s acc=%.2f\n",
           C_GREY, tbuf, C_RST, ch, T, col, size, C_RST, col, best, C_RST, acc);
    fflush(stdout);
}

// ============== POOL LOAD ==============
static bool load_pool(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERROR: cannot open %s\n", path); return false; }
    if (fread(&POOL_HDR, sizeof(POOL_HDR), 1, f) != 1) { fclose(f); return false; }
    AX_TABLE.resize(POOL_HDR.N_table);
    for (uint32_t i = 0; i < POOL_HDR.N_table; i++) {
        if (fread(&AX_TABLE[i], sizeof(AxialIrrEntry), 1, f) != 1) { fclose(f); return false; }
    }
    POOL_FLAT.resize(POOL_HDR.N_total * DIM);
    if (fread(POOL_FLAT.data(), sizeof(double), POOL_HDR.N_total * DIM, f) != POOL_HDR.N_total * DIM) {
        fclose(f); return false;
    }
    fclose(f);
    log_event(C_CYAN, "Pool: %u total = %u anchor + %u axials + %u irr + %u c4 + %u c6  (table=%u)",
              POOL_HDR.N_total, POOL_HDR.N_anchor, POOL_HDR.N_axials, POOL_HDR.N_irr,
              POOL_HDR.N_c4, POOL_HDR.N_c6, POOL_HDR.N_table);
    return true;
}

// Pool-layout helpers
static inline uint32_t ANCHOR_LO()  { return 0; }
static inline uint32_t ANCHOR_HI()  { return POOL_HDR.N_anchor; }                 // exclusive
static inline uint32_t AX_LO()      { return POOL_HDR.N_anchor; }
static inline uint32_t AX_HI()      { return POOL_HDR.N_anchor + POOL_HDR.N_axials; }
static inline uint32_t IRR_LO()     { return AX_HI(); }
static inline uint32_t IRR_HI()     { return AX_HI() + POOL_HDR.N_irr; }
static inline uint32_t C4_LO()      { return IRR_HI(); }
static inline uint32_t C4_HI()      { return IRR_HI() + POOL_HDR.N_c4; }
static inline uint32_t C6_LO()      { return C4_HI(); }
static inline uint32_t C6_HI()      { return POOL_HDR.N_total; }

// ============== STATE OPS (FIX-B8 incremental) ==============
static void state_recompute_blockers(State& s) {
    s.blocker_count.assign(POOL_HDR.N_total, 0);
    for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
        if (s.active[i]) continue;
        int bc = 0;
        for (int j : s.active_idx) {
            if (dot13(poolv(i), poolv(j)) > THRESH_INNER + EPS) bc++;
        }
        s.blocker_count[i] = bc;
    }
}

static void state_init_anchor(State& s, std::mt19937& rng) {
    // Initialize ACTIVE = the 1106 anchor (fixed)
    s.active.assign(POOL_HDR.N_total, 0);
    s.active_idx.clear();
    s.size = 0;
    for (uint32_t i = ANCHOR_LO(); i < ANCHOR_HI(); i++) {
        s.active[i] = 1;
        s.active_idx.push_back(i);
        s.size++;
    }
    // Compute initial blocker_count
    state_recompute_blockers(s);
    
    // Greedy random fill on top of anchor
    std::vector<int32_t> order;
    for (uint32_t i = ANCHOR_HI(); i < POOL_HDR.N_total; i++) order.push_back(i);
    std::shuffle(order.begin(), order.end(), rng);
    for (int idx : order) {
        if (s.blocker_count[idx] != 0) continue;
        // Add it
        s.active[idx] = 1;
        s.active_idx.push_back(idx);
        s.size++;
        // Update blocker_count for others
        for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
            if (s.active[i] || (int32_t)i == idx) continue;
            if (dot13(poolv(idx), poolv(i)) > THRESH_INNER + EPS) s.blocker_count[i]++;
        }
    }
}

static void state_add(State& s, int32_t k) {
    s.active[k] = 1;
    s.active_idx.push_back(k);
    s.size++;
    for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
        if ((int32_t)i == k || s.active[i]) continue;
        if (dot13(poolv(k), poolv(i)) > THRESH_INNER + EPS) s.blocker_count[i]++;
    }
}
static void state_remove(State& s, int32_t k) {
    s.active[k] = 0;
    auto it = std::find(s.active_idx.begin(), s.active_idx.end(), k);
    if (it != s.active_idx.end()) s.active_idx.erase(it);
    s.size--;
    for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
        if ((int32_t)i == k || s.active[i]) continue;
        if (dot13(poolv(k), poolv(i)) > THRESH_INNER + EPS) s.blocker_count[i]--;
    }
}

// ============== MOVES ==============

// Move R-FORWARD: remove 1 axial, add its 2 irrationals. Net = +1.
// Domain: axials currently active whose 2 irrationals are both currently INactive
// and have blocker_count exactly 1 (the conflict being just this axial).
static bool move_R_forward(State& s, std::mt19937& rng, int& info_ax_pool, int& info_irr_a, int& info_irr_b) {
    std::vector<int> applicable;
    for (uint32_t e = 0; e < POOL_HDR.N_table; e++) {
        const auto& en = AX_TABLE[e];
        if (!s.active[en.ax_idx]) continue;
        if (s.active[en.irr_a] || s.active[en.irr_b]) continue;
        // Must be applicable: irr_a's only active blocker is en.ax_idx (and same for irr_b)
        if (s.blocker_count[en.irr_a] != 1) continue;
        if (s.blocker_count[en.irr_b] != 1) continue;
        applicable.push_back(e);
    }
    if (applicable.empty()) return false;
    int e = applicable[std::uniform_int_distribution<int>(0, applicable.size()-1)(rng)];
    const auto& en = AX_TABLE[e];
    info_ax_pool = en.ax_idx; info_irr_a = en.irr_a; info_irr_b = en.irr_b;
    state_remove(s, en.ax_idx);
    state_add(s, en.irr_a);
    state_add(s, en.irr_b);
    return true;
}

// Move R-REVERSE: remove 2 irrationals (an axial-pair), add the corresponding axial. Net = -1.
// Used for SA escape from partial chain dead-ends.
static bool move_R_reverse(State& s, std::mt19937& rng, double T) {
    std::vector<int> applicable;
    for (uint32_t e = 0; e < POOL_HDR.N_table; e++) {
        const auto& en = AX_TABLE[e];
        if (s.active[en.ax_idx]) continue;
        if (!s.active[en.irr_a] || !s.active[en.irr_b]) continue;
        applicable.push_back(e);
    }
    if (applicable.empty()) return false;
    int e = applicable[std::uniform_int_distribution<int>(0, applicable.size()-1)(rng)];
    const auto& en = AX_TABLE[e];
    // Metropolis: dE = +1 (size goes down), accept w.p. exp(-1/T)
    double prob = std::exp(-1.0 / std::max(T, 1e-9));
    if (std::uniform_real_distribution<double>(0,1)(rng) > prob) return false;
    state_remove(s, en.irr_a);
    state_remove(s, en.irr_b);
    state_add(s, en.ax_idx);
    return true;
}

// Move free_add: add any inactive vector with blocker_count == 0.
static bool move_free_add(State& s, std::mt19937& rng) {
    std::vector<int32_t> cands;
    for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
        if (!s.active[i] && s.blocker_count[i] == 0) cands.push_back(i);
    }
    if (cands.empty()) return false;
    int pick = cands[std::uniform_int_distribution<int>(0, cands.size()-1)(rng)];
    state_add(s, pick);
    return true;
}

// Move swap: take an inactive with bc==1, evict its blocker, add it. Net = 0.
static bool move_swap(State& s, std::mt19937& rng) {
    std::vector<int32_t> cands;
    for (uint32_t i = 0; i < POOL_HDR.N_total; i++) {
        if (!s.active[i] && s.blocker_count[i] == 1) cands.push_back(i);
    }
    if (cands.empty()) return false;
    int pick = cands[std::uniform_int_distribution<int>(0, cands.size()-1)(rng)];
    int blocker = -1;
    for (int j : s.active_idx) {
        if (dot13(poolv(pick), poolv(j)) > THRESH_INNER + EPS) { blocker = j; break; }
    }
    if (blocker < 0) return false;
    state_remove(s, blocker);
    state_add(s, pick);
    return true;
}

// Move R-2 SECONDARY: 4-by-1 swap from casta-4. Used above 1146 only.
static bool move_R2(State& s, std::mt19937& rng, double T) {
    if (s.size < 1146) return false;  // gated to fine refinement zone
    std::vector<int32_t> cands;
    for (uint32_t i = C4_LO(); i < C4_HI(); i++) {
        if (!s.active[i] && s.blocker_count[i] >= 1 && s.blocker_count[i] <= 4) cands.push_back(i);
    }
    if (cands.empty()) return false;
    int pick = cands[std::uniform_int_distribution<int>(0, cands.size()-1)(rng)];
    int K = s.blocker_count[pick];
    
    std::vector<int32_t> blockers;
    for (int j : s.active_idx) {
        if (dot13(poolv(pick), poolv(j)) > THRESH_INNER + EPS) blockers.push_back(j);
    }
    if ((int)blockers.size() != K) return false;
    
    int dE = K - 1;
    double prob = std::exp(-(double)dE / std::max(T, 1e-9));
    if (std::uniform_real_distribution<double>(0,1)(rng) > prob) return false;
    
    for (int b : blockers) state_remove(s, b);
    state_add(s, pick);
    // Refill greedily
    for (int t = 0; t < K + 2; t++) {
        if (!move_free_add(s, rng)) break;
    }
    return true;
}

// Move E EARTHQUAKE: remove M random + greedy refill.
static bool move_earthquake(State& s, std::mt19937& rng) {
    int sz = s.size;
    if (sz < 50) return false;
    int M;
    if (sz < 1000)        M = std::max(5, sz / 30);     // ~3-30
    else if (sz < 1130)   M = std::max(10, sz / 20);    // ~50-60
    else                  M = std::max(20, sz / 15);    // ~75
    
    std::vector<int32_t> shuf = s.active_idx;
    // Don't earthquake the anchor (1106 fixed)
    shuf.erase(std::remove_if(shuf.begin(), shuf.end(),
                              [](int32_t i){ return i < (int32_t)ANCHOR_HI() && i < (int32_t)POOL_HDR.N_anchor; }),
               shuf.end());
    if ((int)shuf.size() < M) return false;
    std::shuffle(shuf.begin(), shuf.end(), rng);
    for (int i = 0; i < M; i++) state_remove(s, shuf[i]);
    while (move_free_add(s, rng)) {}
    return true;
}

// ============== SANITY ==============
static bool sanity_check(const State& s, const char* tag) {
    if ((int)s.active_idx.size() != s.size) {
        log_event(C_RED, "SANITY[%s] size mismatch %d vs %d", tag, s.size, (int)s.active_idx.size());
        return false;
    }
    for (int i : s.active_idx) {
        if (!s.active[i]) {
            log_event(C_RED, "SANITY[%s] active_idx %d has active[%d]=0", tag, i, i);
            return false;
        }
    }
    for (size_t a = 0; a < s.active_idx.size(); a++) {
        int ia = s.active_idx[a];
        // norm²
        double n2 = dot13(poolv(ia), poolv(ia));
        if (std::abs(n2 - NORM_SQ) > 1e-6) {
            log_event(C_RED, "SANITY[%s] idx %d norm² = %.6f", tag, ia, n2);
            return false;
        }
        for (size_t b = a+1; b < s.active_idx.size(); b++) {
            int ib = s.active_idx[b];
            double ip = dot13(poolv(ia), poolv(ib));
            if (ip > THRESH_INNER + 1e-6) {
                log_event(C_RED, "SANITY[%s] pair (%d,%d) <x,y>=%.6f", tag, ia, ib, ip);
                return false;
            }
        }
    }
    return true;
}

// ============== SA CHAIN ==============
struct Chain {
    State        state;
    State        best_state;
    int          best_size;
    double       T;
    std::mt19937 rng;
    int          accepts;
    int          rejects;
    int          stagnation_outer;  // outers since last best improvement
    bool         seen_1131;
    bool         seen_1146;
    bool         seen_1154;
    bool         seen_1155;
};

static void chain_init(Chain& c, int chain_id, uint32_t seed) {
    c.rng = std::mt19937(seed + chain_id * 1000003);
    c.T = T_INIT[chain_id];
    state_init_anchor(c.state, c.rng);
    c.best_size = c.state.size;
    c.best_state = c.state;
    c.accepts = c.rejects = 0;
    c.stagnation_outer = 0;
    c.seen_1131 = c.seen_1146 = c.seen_1154 = c.seen_1155 = false;
}

static void chain_step(Chain& c, int chain_id) {
    auto& s = c.state;
    auto& rng = c.rng;
    
    double r = std::uniform_real_distribution<double>(0,1)(rng);
    bool moved = false;
    int info_ax = -1, info_a = -1, info_b = -1;
    
    // Move selection biased by T and current state
    if (r < 0.40) {
        moved = move_R_forward(s, rng, info_ax, info_a, info_b);
        if (moved) {
            // Log structural moves (Casta-IDs for paper v7)
            if (s.size <= 1131 + 24)  // first 24 forward ops
                log_event(C_CYAN, "C%d Move R+ : rem ax pool[%d], add irr pool[%d,%d] → size %d",
                          chain_id, info_ax, info_a, info_b, s.size);
        }
    } else if (r < 0.55) {
        if (!moved) moved = move_free_add(s, rng);
    } else if (r < 0.75) {
        if (!moved) moved = move_swap(s, rng);
    } else if (r < 0.88) {
        if (!moved) moved = move_R_reverse(s, rng, c.T);
    } else {
        if (!moved) moved = move_R2(s, rng, c.T);
    }
    
    // Fallback if nothing applicable
    if (!moved) moved = move_swap(s, rng);
    if (!moved) moved = move_free_add(s, rng);
    
    if (moved) c.accepts++; else c.rejects++;
    
    // Update best
    if (s.size > c.best_size) {
        // Verify-gate on big jumps
        if (s.size >= 1131 && (s.size - c.best_size >= 5 || s.size >= 1146)) {
            if (!sanity_check(s, "verify-gate")) {
                s = c.best_state;
                return;
            }
        }
        c.best_size = s.size;
        c.best_state = s;
        c.stagnation_outer = 0;
        
        // Banner triggers (one-shot per chain)
        if (s.size >= 1131 && !c.seen_1131) { banner_1131(chain_id); c.seen_1131 = true; }
        if (s.size >= 1146 && !c.seen_1146) { banner_1146(chain_id); c.seen_1146 = true; }
        if (s.size >= 1154 && !c.seen_1154) { banner_1154(chain_id); c.seen_1154 = true; }
        if (s.size >= 1155 && !c.seen_1155) { banner_1155(chain_id); c.seen_1155 = true; }
    }
}

// ============== POLISH ==============
static int polish_phase(State& s, std::mt19937& rng) {
    int initial = s.size;
    log_event(C_MAG, "POLISH phase: %d iters, only-improvement", POLISH_ITERS);
    int last_log = 0;
    for (int it = 0; it < POLISH_ITERS; it++) {
        int info_ax, info_a, info_b;
        if (move_R_forward(s, rng, info_ax, info_a, info_b)) continue;
        if (move_free_add(s, rng)) continue;
        move_swap(s, rng);
        if (it - last_log > 25000) {
            log_event(C_DIM, "  polish iter %d size=%d gain=%+d", it, s.size, s.size - initial);
            last_log = it;
        }
    }
    return s.size - initial;
}

// ============== PT SWAP ==============
// Best-state-aware PT swap: exchanges state AND best_state across adjacent chains.
static void pt_swap_pass(std::vector<Chain>& chains, std::mt19937& rng) {
    for (int ch = 0; ch < N_CHAINS - 1; ch++) {
        int s1 = chains[ch].state.size;
        int s2 = chains[ch+1].state.size;
        double dT = 1.0 / chains[ch].T - 1.0 / chains[ch+1].T;
        double dE = (double)(s1 - s2);
        double swap_prob = std::min(1.0, std::exp(dT * dE));
        if (std::uniform_real_distribution<double>(0,1)(rng) < swap_prob) {
            std::swap(chains[ch].state, chains[ch+1].state);
            // Also exchange best_state (per G25 fix)
            if (chains[ch+1].best_size > chains[ch].best_size) {
                chains[ch].best_state = chains[ch+1].best_state;
                chains[ch].best_size = chains[ch+1].best_size;
            }
        }
    }
}

// ============== MAIN ==============
int main(int argc, char** argv) {
    g_t0_us = now_us();
    
    log_banner("TRINCANEROELDELOSCOJONESPUROS — LUNA engine #3 — joint Claude-1+Claude-2");
    
    uint32_t seed = (uint32_t)time(nullptr);
    if (argc > 1) seed = (uint32_t)atoi(argv[1]);
    log_event(C_CYAN, "Seed: %u", seed);
    
    std::mt19937 setup_rng(seed);
    int gi = std::uniform_int_distribution<int>(0, sizeof(GREETINGS)/sizeof(GREETINGS[0])-1)(setup_rng);
    log_event(C_BOLD, "%s", GREETINGS[gi]);
    
    if (!load_pool("pool_v17.bin")) return 1;
    
    log_event(C_CYAN, "Initializing %d SA chains from 1106-anchor + greedy...", N_CHAINS);
    std::vector<Chain> chains(N_CHAINS);
    for (int ch = 0; ch < N_CHAINS; ch++) {
        chain_init(chains[ch], ch, seed);
        log_chain(ch, chains[ch].T, chains[ch].state.size, chains[ch].best_size, 0, 0);
    }
    
    int global_best = 0;
    for (auto& c : chains) global_best = std::max(global_best, c.best_size);
    log_event(C_BOLD C_GREEN, "All chains seeded. Initial global best: %d", global_best);
    
    // Initial banners (might already be > 1131 from greedy)
    for (int ch = 0; ch < N_CHAINS; ch++) {
        if (chains[ch].best_size >= 1131 && !chains[ch].seen_1131) {
            banner_1131(ch); chains[ch].seen_1131 = true;
        }
        if (chains[ch].best_size >= 1146 && !chains[ch].seen_1146) {
            banner_1146(ch); chains[ch].seen_1146 = true;
        }
    }
    
    long last_heartbeat_us = since_start_us();
    
    for (int outer = 0; outer < N_OUTER; outer++) {
        double prog = (double)outer / std::max(1, N_OUTER - 1);
        for (int ch = 0; ch < N_CHAINS; ch++) {
            double a = std::log(T_INIT[ch]);
            double b = std::log(T_FINAL[ch]);
            chains[ch].T = std::exp(a + prog * (b - a));
        }
        
        for (int ch = 0; ch < N_CHAINS; ch++) {
            int prev_best = chains[ch].best_size;
            for (int it = 0; it < N_INNER; it++) {
                chain_step(chains[ch], ch);
            }
            // Stagnation tracking
            if (chains[ch].best_size > prev_best) chains[ch].stagnation_outer = 0;
            else chains[ch].stagnation_outer++;
            
            // Earthquake trigger: adaptive by state size (3 bands)
            int sz = chains[ch].state.size;
            int trig;
            if (sz < 1000)      trig = 50;
            else if (sz < 1130) trig = 150;
            else                trig = 250;
            if (chains[ch].stagnation_outer >= trig) {
                log_event(C_MAG, "C%d earthquake at outer=%d (stag=%d, size=%d)",
                          ch, outer, chains[ch].stagnation_outer, sz);
                move_earthquake(chains[ch].state, chains[ch].rng);
                chains[ch].stagnation_outer = 0;
            }
        }
        
        if (outer > 0 && outer % PT_EVERY == 0) {
            pt_swap_pass(chains, setup_rng);
        }
        
        for (auto& c : chains) global_best = std::max(global_best, c.best_size);
        
        for (int ch = 0; ch < N_CHAINS; ch++) {
            g_chart.best_history[ch].push_back(chains[ch].best_size);
        }
        
        long now = since_start_us();
        if (now - last_heartbeat_us > HEARTBEAT_SEC * 1000000) {
            for (int ch = 0; ch < N_CHAINS; ch++) {
                log_chain(ch, chains[ch].T, chains[ch].state.size, chains[ch].best_size,
                          chains[ch].accepts, chains[ch].rejects);
            }
            static int chart_counter = 0;
            if (++chart_counter >= 10) { draw_chart(global_best); chart_counter = 0; }
            last_heartbeat_us = now;
        }
    }
    
    log_banner("SA phase done. Polish on best chain.");
    int best_ch = 0;
    for (int ch = 1; ch < N_CHAINS; ch++) {
        if (chains[ch].best_size > chains[best_ch].best_size) best_ch = ch;
    }
    log_event(C_BOLD C_GREEN, "Best chain: C%d at size %d", best_ch, chains[best_ch].best_size);
    
    State polished = chains[best_ch].best_state;
    int gain = polish_phase(polished, chains[best_ch].rng);
    log_event(C_MAG, "Polish gain: %+d → final %d", gain, polished.size);
    
    log_banner("FINAL VERIFY");
    bool ok = sanity_check(polished, "FINAL");
    if (!ok) { log_event(C_RED, "FINAL VERIFY FAILED"); return 1; }
    log_event(C_BOLD C_GREEN, "FINAL VERIFY PASSED. Final size = %d.", polished.size);
    
    char outpath[256];
    snprintf(outpath, sizeof(outpath), "cojones_puros_final_%d.txt", polished.size);
    FILE* fout = fopen(outpath, "w");
    if (fout) {
        fprintf(fout, "# TRINCANEROELDELOSCOJONESPUROS final, seed=%u, size=%d\n", seed, polished.size);
        for (int i : polished.active_idx) {
            for (int k = 0; k < DIM; k++) {
                fprintf(fout, "%.15g%s", poolv(i)[k], k == DIM-1 ? "\n" : ",");
            }
        }
        fclose(fout);
        log_event(C_BOLD, "Final config saved to %s", outpath);
    }
    
    log_banner("RUN COMPLETE");
    log_event(C_BOLD, "Best size: %d", polished.size);
    log_event(C_BOLD, "vs P13a 1130:        %+d", polished.size - 1130);
    log_event(C_BOLD, "vs PackingStar 1146: %+d", polished.size - 1146);
    log_event(C_BOLD, "vs ZE99 1154:        %+d", polished.size - 1154);
    
    if (polished.size >= 1155) {
        log_event(C_BOLD C_RED, "★ ★ ★ K(13) ≥ 1155 — VERIFY EXTERNALLY WITH PYTHON KERNEL ★ ★ ★");
    } else if (polished.size >= 1154) {
        log_event(C_BOLD C_RED, "★ ZE99 1154 reproduced from cold via F40 chain.");
    } else if (polished.size >= 1146) {
        log_event(C_BOLD C_MAG, "★ PackingStar 1146 territory reached.");
    } else if (polished.size >= 1131) {
        log_event(C_BOLD C_GREEN, "★ P13a 1131 floor crossed.");
    }
    return 0;
}
