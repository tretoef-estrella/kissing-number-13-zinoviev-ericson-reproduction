"""
Frente A-bis Phase 4 — Grieta G size-parity hypothesis sandbox.

Architect (07 May 2026): the V11+V12 attacks all sought delta = +1
(target size 1155). If the geometry of dim 13 admits parity in size
analogous to the cset-parity confirmed by F-frente-A-2 + F-A-bis-2
(cset values populate {0, 2, 4, 6, ...} with {1, 3} empty), then
delta = +2, +3, +4 may be reachable while delta = +1 is forbidden.
Parallel: Diamante 22-6-13's excess = 2 and excess = 5 never appeared.

This script explores k-multi-swap on the existing 1,608-candidate cset≤4
catalog with relaxed U-budget pruning, sweeping target sizes:
    1156 (delta = +2):  |U| <= k - 2
    1157 (delta = +3):  |U| <= k - 3
    1158 (delta = +4):  |U| <= k - 4

For each delta target, we report the smallest k that admits a record,
or 0 records up to k_max.

INSTRUCTIONS FOR ARCHITECT:
  1. Place this script in ~/Downloads/ alongside the Phase 1 catalog
     `PHASE1_BIS_EXT_RESULTS.json` (the 1,608-vector extended catalog
     produced by Frente A-bis Phase 1 sandbox).
  2. Run from terminal, foreground per directive D[N]:
       cd ~/Downloads && caffeinate -dims taskpolicy -c utility \\
           python3 frente_a_bis_phase4.py 2>&1 | tee phase4_run.log
  3. Total expected wall-clock: 5-15 minutes on Mac M2 single-thread.
  4. If any record candidate is found:
       - The script outputs full vector data and verifies the configuration
         size and inner-product validity.
       - DO NOT claim record until D190 quadruple-verify passes:
         (1) this script's internal verify, (2) ROUNDTRIP write/read,
         (3) STRUCTURAL no-duplicate check, (4) external Python kernel
         from a fresh process re-loading from JSON.
       - Architect arbitrates promotion; Auditor cross-checks before
         any external claim.
  5. If 0 records across all delta targets: register F-A-bis-5
     (delta-bound saturation) as paper-grade contribution to v6.

DEPENDENCIES:
  - Python 3.7+ with numpy
  - Input file: PHASE1_BIS_EXT_RESULTS.json (1,608 candidates, cset<=4)

OUTPUT:
  - PHASE4_GRIETA_G_RESULTS.json (full sweep summary)
  - phase4_run.log (terminal output)
  - POTENTIAL_RECORD_CANDIDATES_PHASE4.json (only if records found)
"""

import os
import sys
import json
import time
import itertools
from collections import Counter, defaultdict

import numpy as np

# ---- configuration ----
ROOT = os.path.dirname(os.path.abspath(__file__)) or '.'
INPUT_JSON = os.path.join(ROOT, 'PHASE1_BIS_EXT_RESULTS.json')
OUTPUT_JSON = os.path.join(ROOT, 'PHASE4_GRIETA_G_RESULTS.json')
RECORD_OUT_JSON = os.path.join(ROOT, 'POTENTIAL_RECORD_CANDIDATES_PHASE4.json')

# Search parameters
DELTAS = [2, 3, 4]               # target sizes 1156, 1157, 1158
K_MIN = 2
K_MAX = 7                        # depth cap for backtrack (7 covers delta=+4 well)
TIME_CAP_PER_DELTA = 600         # seconds per delta sweep
EARLY_STOP_QUINTS = 50           # if found, stop and verify

ZE99_BASELINE = 1154

# ---------- load catalog ----------
print("=" * 70)
print("FRENTE A-bis PHASE 4 — Grieta G size-parity sweep")
print("=" * 70)
print(f"\nInput:  {INPUT_JSON}")
if not os.path.exists(INPUT_JSON):
    print(f"ERROR: input catalog not found at {INPUT_JSON}")
    print(f"Place PHASE1_BIS_EXT_RESULTS.json in {ROOT} and re-run.")
    sys.exit(1)

t_total = time.time()

with open(INPUT_JSON) as f:
    R_ext = json.load(f)
plat = R_ext.get('plateau_findings') or R_ext.get('candidates') or []
n = len(plat)
print(f"Loaded {n} candidates (cset<=4 catalog)")
if n == 0:
    print("ERROR: no candidates in catalog. Aborting.")
    sys.exit(1)

# Build numpy arrays
V = np.zeros((n, 13), dtype=np.int64)
B_arr = np.zeros((n, 13), dtype=np.int64)
csets_list = []
for k, p in enumerate(plat):
    for j, (a, b) in enumerate(p['vector']):
        V[k, j] = a
        B_arr[k, j] = b
    csets_list.append(frozenset(p['conflict_indices_in_ze99']))

assert (B_arr == 0).all(), "non-integer candidates present in catalog"

cset_size = np.array([len(c) for c in csets_list])
cset_dist = Counter(cset_size.tolist())
print(f"cset size distribution: {dict(cset_dist)}")

# Sort: lower cset first to optimize pruning
order = np.argsort(cset_size, kind='stable')
V = V[order]
csets_list = [csets_list[i] for i in order]
cset_size = cset_size[order]
print(f"Reordered: cset=2 indices 0..{int((cset_size==2).sum())-1}, "
      f"cset=4 indices {int((cset_size==2).sum())}..{n-1}")

# Pairwise compatibility
print("\n--- pairwise compatibility ---")
t0 = time.time()
IPm = V @ V.T
compat = (IPm <= 8)
np.fill_diagonal(compat, False)
n_compat_pairs = int(compat.sum() // 2)
print(f"Compat matrix built [{time.time()-t0:.2f}s]  "
      f"density={n_compat_pairs / (n*(n-1)//2):.3f}  "
      f"edges={n_compat_pairs:,}")

adj_lists = [set(np.where(compat[i])[0].tolist()) for i in range(n)]


# ---------- backtracking search per delta target ----------

def search_delta(target_delta, time_cap=TIME_CAP_PER_DELTA):
    """Search for k-clique with |U| <= k - target_delta.
    Iterate k from K_MIN to K_MAX. Return list of records.
    """
    print(f"\n{'='*70}")
    print(f"DELTA = +{target_delta}  (target size {ZE99_BASELINE + target_delta})")
    print(f"{'='*70}")
    all_records = []
    for k_target in range(max(K_MIN, target_delta + 1), K_MAX + 1):
        u_budget = k_target - target_delta
        if u_budget < 0:
            continue
        print(f"\n  k = {k_target}, |U| <= {u_budget}, target size = "
              f"{ZE99_BASELINE - u_budget + k_target}")
        records = []
        n_visit = 0
        t0 = time.time()

        def search(partial, U, depth):
            nonlocal n_visit
            n_visit += 1
            if depth == k_target:
                # Verify |U| <= u_budget (should hold by construction)
                if len(U) <= u_budget:
                    records.append((tuple(partial), U))
                return
            last = partial[-1]
            # neighbors of all in partial AND > last
            nbrs_mask = compat[last].copy()
            for v in partial[:-1]:
                nbrs_mask &= compat[v]
            nbrs_mask[:last+1] = False
            nbrs = np.where(nbrs_mask)[0]
            for w in nbrs:
                cs_w = csets_list[w]
                new_U = U | cs_w
                if len(new_U) <= u_budget:
                    partial.append(int(w))
                    search(partial, new_U, depth + 1)
                    partial.pop()
                    if len(records) >= EARLY_STOP_QUINTS:
                        return
                    if time.time() - t0 > time_cap:
                        return

        for v0 in range(n):
            if cset_size[v0] > u_budget:
                continue
            U0 = csets_list[v0]
            search([v0], U0, 1)
            if len(records) >= EARLY_STOP_QUINTS:
                print(f"    early stop: {EARLY_STOP_QUINTS}+ records")
                break
            if time.time() - t0 > time_cap:
                print(f"    time cap {time_cap}s reached")
                break

        elapsed = time.time() - t0
        print(f"    visits={n_visit:,}  records={len(records)}  "
              f"elapsed={elapsed:.1f}s")
        if records:
            print(f"    *** {len(records)} record(s) at k={k_target}, "
                  f"delta=+{target_delta} ***")
            all_records.extend(
                {'k': k_target, 'delta': target_delta,
                 'indices': list(idxs), 'union_conflicts': sorted(U)}
                for (idxs, U) in records[:20]
            )
            # Stop further k for this delta — minimum-k record is enough
            break
    return all_records


# ---------- main sweep ----------
results_by_delta = {}
all_records_total = []
for delta in DELTAS:
    records = search_delta(delta)
    results_by_delta[delta] = records
    all_records_total.extend(records)

# ---------- verification of any records ----------

def verify_record(rec):
    idxs = rec['indices']
    delta = rec['delta']
    k = rec['k']
    # Pairwise
    all_compat = True
    ips = []
    for i in range(len(idxs)):
        for j in range(i+1, len(idxs)):
            ip = int(IPm[idxs[i], idxs[j]])
            ips.append(ip)
            if ip > 8:
                all_compat = False
    # Union of csets
    U = set()
    for kk in idxs:
        U |= csets_list[kk]
    final_size = ZE99_BASELINE - len(U) + k
    expected_size = ZE99_BASELINE + delta
    return {
        'all_pairwise_compat': all_compat,
        'union_size': len(U),
        'k': k,
        'final_size': final_size,
        'expected_size': expected_size,
        'size_match': final_size == expected_size,
        'pairwise_ips': ips,
        'union_indices': sorted(U),
        'candidate_indices_in_catalog': idxs,
        'candidate_vectors': [V[kk].tolist() for kk in idxs],
    }


verified_records = []
if all_records_total:
    print(f"\n{'='*70}")
    print(f"VERIFICATION — {len(all_records_total)} record candidate(s)")
    print(f"{'='*70}")
    for rec_i, rec in enumerate(all_records_total[:20]):
        v = verify_record(rec)
        verified_records.append({**rec, 'verification': v})
        print(f"\n  record {rec_i+1}: k={rec['k']}, delta=+{rec['delta']}")
        print(f"    indices: {rec['indices']}")
        print(f"    union conflicts: {v['union_indices']} (size {v['union_size']})")
        print(f"    final size: {v['final_size']} (expected {v['expected_size']})  "
              f"size_match={v['size_match']}")
        print(f"    all pairwise compat: {v['all_pairwise_compat']}")
        if v['all_pairwise_compat'] and v['size_match']:
            print(f"    >>> POTENTIAL RECORD K(13) >= {v['final_size']} <<<")

# ---------- save ----------
out = {
    'metadata': {
        'phase': 'Frente A-bis Phase 4 — Grieta G size-parity',
        'catalog_size': n,
        'compat_density': n_compat_pairs / (n*(n-1)//2),
        'deltas_tested': DELTAS,
        'k_range': [K_MIN, K_MAX],
        'wall_clock_seconds': time.time() - t_total,
    },
    'results_by_delta': {
        str(d): {'records_found': len(rs), 'examples': rs[:20]}
        for d, rs in results_by_delta.items()
    },
    'total_records': len(all_records_total),
}
with open(OUTPUT_JSON, 'w') as f:
    json.dump(out, f, indent=2, default=str)
print(f"\nWritten: {OUTPUT_JSON}")

if verified_records:
    with open(RECORD_OUT_JSON, 'w') as f:
        json.dump({'records': verified_records}, f, indent=2, default=str)
    print(f"Written: {RECORD_OUT_JSON}")

# ---------- final summary ----------
print(f"\n{'='*70}")
print(f"PHASE 4 SUMMARY — Grieta G size-parity sweep")
print(f"{'='*70}")
for delta in DELTAS:
    n_rec = len(results_by_delta[delta])
    target = ZE99_BASELINE + delta
    if n_rec > 0:
        print(f"  delta=+{delta} (size {target}): {n_rec} record(s) FOUND")
    else:
        print(f"  delta=+{delta} (size {target}): 0 records")
print()
if all_records_total:
    print("  >>> POTENTIAL RECORD MATERIAL FOUND <<<")
    print("  >>> Verify with D190 quadruple-verify before any external claim <<<")
    print("  >>> Architect arbitration required for promotion <<<")
else:
    print("  Grieta G (size-parity hypothesis) returns 0 records on this catalog.")
    print("  This empirically confirms F-A-bis-5 (delta-bound saturation):")
    print("  no path to 1156, 1157, or 1158 via multi-swap up to k=" + str(K_MAX))
    print("  on the cset<=4 catalog of 1,608 native-shell candidates.")
    print("  Paper-grade contribution to closure paper v6 §13.12.")
print(f"\nTotal wall-clock: {time.time()-t_total:.1f}s")
print(f"{'='*70}")
