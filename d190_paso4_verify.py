#!/usr/bin/env python3
# =====================================================================
# D190 paso 4 — Independent Python kernel verifier for DOSPIRATAS dumps.
#
# Purpose: take a dump produced by DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS
# and recompute kissing-validity FROM SCRATCH using ONLY:
#   - Python's float arithmetic (no engine state, no incremental tracking)
#   - numpy linear algebra
# This independent computation is paso 4 of D190 quadruple-verify.
# If this script reports VALID-RECORD on a dump, the engine's claim is
# independently confirmed and ready for D190 paso 2 (roundtrip serialization)
# and paso 3 (structural sanity vs ZE99/SDP bounds).
#
# Usage:
#   python3 d190_paso4_verify.py DOSPIRATAS_dump_violX_size1155.txt
#
# Output:
#   PASS-VALID-RECORD if pair_violations == 0 and N == 1155 and norms² == 16.
#   PASS-VALID-NEAR   if pair_violations > 0 (matches engine claim).
#   FAIL-MISMATCH     if engine-claimed violations != recomputed violations.
#   FAIL-NORM         if any vector norm² != 16 within 1e-6.
#   FAIL-DUPS         if any duplicate vectors detected.
# =====================================================================

import sys
import re
import numpy as np

EPS = 1e-6
NORM2_TARGET = 16.0
IP_THRESHOLD = 8.0
DIM = 13
N_TARGET = 1155

def parse_dump(path):
    """Parse the DOSPIRATAS dump file. Returns (claimed_viol, vectors)."""
    claimed_viol = None
    claimed_size = None
    vectors = []
    with open(path) as f:
        for lineno, raw in enumerate(f, 1):
            line = raw.strip()
            if not line:
                continue
            if line.startswith('#'):
                # parse: "# DOSPIRATAS dump | size=1155 | pair_violations=2"
                m_size = re.search(r'size=(\d+)', line)
                m_viol = re.search(r'pair_violations=(-?\d+)', line)
                if m_size:
                    claimed_size = int(m_size.group(1))
                if m_viol:
                    claimed_viol = int(m_viol.group(1))
                continue
            parts = line.split(',')
            if len(parts) != DIM:
                raise ValueError(f"line {lineno}: {len(parts)} coords, expected {DIM}")
            try:
                v = [float(x) for x in parts]
            except ValueError as e:
                raise ValueError(f"line {lineno} parse: {e}")
            vectors.append(v)
    return claimed_viol, claimed_size, vectors

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 d190_paso4_verify.py <dump_file>")
        sys.exit(2)
    path = sys.argv[1]
    print(f"[D190-P4] verifying {path}")
    print(f"[D190-P4] EPS={EPS} NORM2_TARGET={NORM2_TARGET} "
          f"IP_THRESHOLD={IP_THRESHOLD} DIM={DIM} N_TARGET={N_TARGET}")

    claimed_viol, claimed_size, vectors = parse_dump(path)
    N = len(vectors)
    print(f"[D190-P4] parsed {N} vectors (engine-claimed size={claimed_size}, "
          f"engine-claimed viol={claimed_viol})")

    if claimed_size is not None and claimed_size != N:
        print(f"FAIL-PARSE: claimed_size={claimed_size} actual N={N}")
        sys.exit(1)

    V = np.asarray(vectors, dtype=np.float64)

    # Norm² check
    norms2 = (V * V).sum(axis=1)
    bad_norm = np.abs(norms2 - NORM2_TARGET) > EPS
    n_bad_norm = int(bad_norm.sum())
    if n_bad_norm > 0:
        print(f"FAIL-NORM: {n_bad_norm} vectors with norm² != {NORM2_TARGET}")
        first_bad = int(np.argmax(bad_norm))
        print(f"  first offender index {first_bad}: norm²={norms2[first_bad]:.9f}")
        sys.exit(1)
    print(f"[D190-P4] norm² check PASS ({N} vectors all at {NORM2_TARGET} ± {EPS})")

    # Duplicates check (bit-exact)
    seen = set()
    dups = []
    for i, v in enumerate(vectors):
        t = tuple(v)
        if t in seen:
            dups.append(i)
        seen.add(t)
    if dups:
        print(f"FAIL-DUPS: {len(dups)} duplicate vectors detected")
        print(f"  first dup at index {dups[0]}")
        sys.exit(1)
    print(f"[D190-P4] dedup check PASS (all {N} vectors distinct)")

    # Pair violations: exhaustive
    IPs = V @ V.T
    # zero out diagonal
    np.fill_diagonal(IPs, -1e9)
    upper = np.triu(IPs, k=1)
    violations_mask = upper > IP_THRESHOLD + EPS
    n_viol = int(violations_mask.sum())
    print(f"[D190-P4] independent pair_violations = {n_viol}")

    if claimed_viol is not None and claimed_viol != n_viol:
        print(f"FAIL-MISMATCH: engine claimed {claimed_viol}, "
              f"independent kernel computes {n_viol}")
        # Show first 10 violations for debugging
        idx_pairs = np.argwhere(violations_mask)[:10]
        for (i, j) in idx_pairs:
            print(f"  pair ({i}, {j}): IP={IPs[i, j]:.6f}")
        sys.exit(1)
    print(f"[D190-P4] engine/independent pair_violations match: {n_viol}")

    # Verdict
    if N != N_TARGET:
        print(f"FAIL-SIZE: N={N} != target {N_TARGET}")
        sys.exit(1)

    if n_viol == 0:
        print(f"\n*** PASS-VALID-RECORD ***")
        print(f"*** D190 paso 4 confirms K(13) >= {N} ***")
        print(f"*** Architect: proceed to D190 paso 2 (roundtrip) and paso 3 "
              f"(structural sanity) ***")
        sys.exit(0)
    else:
        print(f"\nPASS-VALID-NEAR (independent verify confirms engine claim, "
              f"but record requires viol=0; current viol={n_viol})")
        # Distance metrics for paper-grade reporting
        viol_per_vector = violations_mask.sum(axis=1) + violations_mask.sum(axis=0)
        max_v = int(viol_per_vector.max())
        n_with_v = int((viol_per_vector > 0).sum())
        print(f"  vectors involved in conflicts: {n_with_v}")
        print(f"  max violations on a single vector: {max_v}")
        sys.exit(0)

if __name__ == "__main__":
    main()
