# Methodology

> This document is a standalone technical primer on the methods Project LUNA used to attack K(13) ≥ 1155. It is self-contained: it does not require reading the full paper to understand the pipeline. For full proofs and sandbox transcripts, see `PAPER_TERMINAL.md`; for the operational arsenal and lessons, see `COJONES_SABIOS_TERMINAL.md`.

---

## 1. Problem statement in operational form

The kissing number K(n) is the maximum number of non-overlapping unit spheres that can simultaneously touch a central unit sphere in ℝⁿ. Equivalently, after scaling so that all kissing vectors have squared norm 16, K(n) is the largest integer N such that there exist v₁, ..., v_N ∈ ℝⁿ with ‖vᵢ‖² = 16 for all i and ⟨vᵢ, vⱼ⟩ ≤ 8 for all i ≠ j. The constraint is tight when ⟨vᵢ, vⱼ⟩ = 8.

For dim 13, the best known lower bound at project closure was **K(13) ≥ 1154** (Zinoviev–Ericson 1999) and the best known upper bound was **K(13) ≤ 2064** (De Laat–Leijenhorst 2024 via quadruple-precision SDP on the Cohn–Elkies linear programming relaxation). The ratio lower/upper = 1.79 makes dim 13 a target with real headroom relative to the dimensions where K(n) is exactly known (1, 2, 3, 4, 8, 24).

The project reformulated K(13) ≥ N as a maximum-independent-set problem on a finite combinatorial graph G_pool: vertices are candidate vectors in a chosen algebraic alphabet at the unit-sphere norm, edges connect pairs with ⟨vᵢ, vⱼ⟩ > 8. The record falls if any pool admits an independent set of size 1155 or larger. The work was: choose pools systematically, search them under disciplined optimization, and document what saturates where.

---

## 2. The pipeline (TRINCANEROELDELOSCOJONESPUROS lineage)

The canonical engine that reproduces ZE99 from cold initialization is `TRINCANEROELDELOSCOJONESPUROS.cpp`. Its design carries forward two independent ancestries:

### 2.1 ADN Diamante (carried from project Diamond)

- **Flat arrays over `std::vector`** for hot-path performance.
- **FIX-B8 incremental updates**: on a swap (vector i out, vector j in), conflict counters are updated in O(N) by walking the row of the precomputed Gram matrix, never recomputing from scratch (O(N²)).
- **"NEVER REJECT GOOD CONFIG" discipline**: a configuration that strictly improves the current best is never rejected by acceptance probability, regardless of temperature.
- **"SALTAR no grindear"**: when a local plateau is detected, the engine jumps (Move E earthquake, configurable trigger by current size) rather than grinding on small local moves.
- **Cryogenic zone (CRYO_THRESH)**: above a configurable threshold (set to 1131, one above the Leech–Sloane P13a floor of 1130), the engine activates cold-only behavior; below, full thermal exploration.

### 2.2 ADN Sobol (carried from project Sobol)

- **Multi-chain simulated annealing** with parallel tempering: four chains with cold-shifted temperature ladder (T_init = 0.20, 0.40, 0.70, 1.10; T_final = 0.04, 0.08, 0.14, 0.22). PT swap every 5 outer iterations.
- **Tabu caps per chain** to prevent oscillation lock at high T.
- **Polish phase** post-SA: pure-descent only-improvement search for local refinement.
- **Sanity-18 suite**: 18 invariants checked on every accept above the floor (norm, kissing threshold, coordinate ranges, alphabet membership, no duplicates, antipodal symmetry preserved, etc.).
- **D190 quadruple-verify protocol** (see §6 below).
- **Cost-probe per move ≤ 80 ms**: any move primitive that takes longer than 80 ms to evaluate is rejected from the engine's vocabulary at design time.

### 2.3 The 24-step deterministic Move R-axial→irrational chain

The structural backbone of the engine. The Zinoviev–Ericson 1154 configuration decomposes as:

- 1106-vector "anchor" derived from Λ₁₃ floor (Leech–Sloane 1971 P13a) — coordinates in ℤ.
- 24 "pure axials" ±4·eᵢ for i ∈ {0, ..., 11} ∪ {12} — coordinates in ℤ.
- 48 "axial-irrationals" of the form (0, ..., 0, ±2√3, 0, ..., 0, ±2) — coordinates in ℤ[√3].
- 288 "diamonds" of the form (±1)¹² ⊕ (±2) — coordinates in ℤ.

The 24-step chain is: starting from 1106-anchor plus the 24 pure axials (= 1130, the P13a floor), perform 24 deterministic gates. Each gate removes one pure axial ±4·eᵢ and adds the two specific axial-irrationals associated with that coordinate, e.g., (0,...,0,±2√3 at position i, 0,...,0,±2 at position 12). Net per gate: −1 + 2 = +1. After 24 gates: 1130 − 24 + 48 = 1154. The 288 diamonds emerge as the maximal additional Λ₁₃-compatible vectors at norm² = 16 with the same coord-12 privilegio. The engine implements this chain deterministically as Move R-axial→irrational FORWARD; the reverse (Move R-axial→irrational REVERSE, −1 net) is used by SA at higher temperature to escape local maxima derived from partial chains.

**Result**: the engine reaches 1154 from a 1106-anchor cold start in approximately 19 seconds on Mac M2 at 25% CPU single-thread. The dump it produces is byte-exact against Cohn's MIT spherical-codes archive (verified by `verify_ze99_1154.py`).

---

## 3. The pool generation question

Pool generation is the first and hardest mathematical question: which subset of ℝ¹³ is enumerated and filtered against the kissing constraint? The project's pools, in order of attack:

- **Pool of TRINCANERO_v1** (graveyard): rational class integer alphabet, 117,962 candidates at norm² = 16. Ceiling reached: 986 < 1130. Lesson preserved: rational-only alphabet cannot reach the laminated lattice floor.
- **Pool of TRINCANEROELDELOSCOJONESPUROS**: 1106-anchor + 24 axials + 48 axial-irrationals + 384 c4 + 1536 c6, with the deterministic axial→irrational table precomputed. Ceiling reached: 1154 = ZE99. Pool saturated at the record.
- **Frente A pool**: 990,390 candidates in ℚ + ℚ√3 at norm² = 24, swept exhaustively. Records found: zero. F-frente-A-1 (coord-12 fixing) and F-frente-A-2 (empty cset interval at norm² = 24) registered as paper-grade.
- **Frente A-bis pool**: native integer shell norm² = 16, multi-swap delta = +1 at k ≤ 5 (V12), then delta ∈ {+2, +3, +4} at k ≤ 7 (V13). Catalog size 1608. Records found: zero. F-A-bis-1..5 registered as paper-grade.
- **Frente B pool (Eisenstein ℤ[ζ₃] independent)**: 56,807,606 candidates in σ(ℤ[ζ₃]^6) ⊕ ℤ via Minkowski embedding, |u|² = 16. Swept in 232 seconds by `PEINAEISENSTEIN-PHASE0` C++ engine. Records found: zero. F-frente-B-1..3 registered as paper-grade.
- **V15 structural deep-dive on ZE99 internal**: Steiner-style 924-block construction, perfect-matching support analysis, horizon-hole tests on pure axials. No new pool; characterization of ZE99 internal structure. F-frente-B-4..6 registered as paper-grade.

Total swept: ~7.13 × 10⁹ candidates across all natural algebraic families.

---

## 4. The sandbox-kill discipline (directive D24)

Before any heavy compute on the Mac, every algorithmic proposal is **sandbox-killed**: tested in a small Python or C++ sandbox where the time cost is minutes, not hours. The discipline has three flavors:

1. **Pool-feasibility sandbox**: estimate pool size, RAM cost, and IP-filter throughput. The HORMONERO fullpool engine was graveyard'd architecturally when the precomputed conflict graph showed a 4.72 GB requirement on an 8 GB Mac.
2. **Move-feasibility sandbox**: test the proposed move primitive on a small (1000-vector) version of the pool. If it does not produce net positive deltas at any temperature, the move is killed before any full-pool engine is compiled.
3. **Verification sandbox**: any claim of a record (size ≥ 1155) must be re-verified in an independent Python kernel before any external announcement. This is `d190_paso4_verify.py`, step 4 of the D190 protocol.

D24 has graveyarded many proposed engines and is a primary reason no false-positive record claim ever reached the Architect.

---

## 5. The naming discipline (directive D26)

Engine names follow a deliberately irreverent convention (cojones puros, peinados, encerados, perfumados) inspired by the sister projects Diamond and Sobol. Contractual names are **earned**, not assigned speculatively:

- An engine name is officially claimed only after the Move primitives it implements survive sandbox-kill (D24).
- An engine name is officially carried into the paper only if the engine produces a paper-grade contribution (a record, or a paper-grade finding under the F-series).
- Engines that fail sandbox-kill have their reserved names **released**: the V14 engine `MONITODEFERIAUSACERACARCELERA` is **NOT BORN** because the Move D-zeta primitive did not survive sandbox-kill against ZE99.
- Reserved names (e.g., `DECAGONOROMANTICOPERSAREYDESHIELOPOLLITOSRAFA` for the unopened ℚ(ζ₁₀) Vein 1 attack) are released into the public archive at project closure. Any researcher who uses them does so under their own banner.

The irreverence is operational: names ridículos no admiten engines mediocres. When an engine finds a record, the name goes into the paper without apology.

---

## 6. The D190 quadruple-verify protocol

Any record claim must survive four independent verifications:

- **Step 1 (in-engine FINAL_VERIFY)**: the engine itself recomputes the configuration's pairwise inner products against the kissing threshold from scratch after declaring a record, without using any incremental state.
- **Step 2 (ROUNDTRIP)**: the engine serializes the configuration to disk, exits, reloads from disk, and recomputes verification. Any discrepancy fails the claim.
- **Step 3 (structural sanity)**: the configuration is checked against the ZE99 invariants (squared norm 16, kissing condition saturated at IP = 8, ZE99 partial-overlap if same size, no F40 axial recycling per F58 universal-sink, no impossible spectrum) and against the De Laat–Leijenhorst upper bound K(13) ≤ 2064.
- **Step 4 (independent Python kernel)**: an external Python script (`d190_paso4_verify.py`) recomputes the configuration's validity from scratch using float arithmetic and numpy linear algebra, with no shared code with the C++ engine. If the Python kernel disagrees with the engine, the claim fails.

No record claim has ever survived all four steps during the project's lifetime (because no engine ever produced a 1155-or-larger configuration). The protocol's calibration was confirmed retrospectively against the ZE99 1154 dump itself, which passes all four steps cleanly.

---

## 7. The both-hats discipline (V15 §-3.6, single-Claude fallback)

The project's normal operation is **dual-Claude**: one Constructor instance, one Auditor instance, both Claude Opus 4.7, with separated roles enforced by the human Architect rather than by code. When the Auditor is unavailable mid-session (rate limits, context exhaustion), the Constructor applies **both-hats discipline**:

1. **Two-pass output**: every operational claim is produced twice. Pass 1 in Constructor mode (proposing, computing). Pass 2 in Auditor mode (challenging the Pass 1 claim from first principles).
2. **Trampa-suspect categories require Pass 2 explicitly**: (a) interpreting graph properties as geometric properties, (b) constructing constraint matrices where a constraint type might be missing, (c) testing intuition-driven hypotheses whose negative results may still teach something, (d) extrapolating from sample to population, (e) reading numerical coincidences as structural.
3. **Self-catches are documented as Auditor-equivalent**: a trap caught during the session and graveyarded with explicit lesson is operationally indistinguishable from an Auditor catch. G33 / G34 / G35 of V15 are self-catches.
4. **Findings produced under both-hats discipline are flagged for Auditor verification on restoration**: F-frente-B-4 / F-frente-B-5 / F-frente-B-6 carry an implicit footnote until verified by the Auditor.
5. **The discipline does not extend session length**: a Constructor under both-hats discipline is slower per turn but does not justify expanding the session beyond what Auditor-supervised work would cover.

This is the canonical AI-research operational pattern for single-instance fallback in dual-instance setups.

---

## 8. Hardware and reproducibility

All compute was performed on a Mac M2 8-core, restricted to single-thread at 25% CPU maximum. No GPU, no cluster, no cloud. Python sandbox computations used numpy and scipy on the same machine. C++ engines compile with `g++ -O3 -march=native -std=c++17 -funroll-loops`. Total wall-clock for the canonical ZE99 reproduction: ~19 seconds. Total wall-clock for the largest sweep (Frente B Eisenstein, 56.8 M candidates): 232 seconds. The full project history fits in a single Mac M2 budget.

This hardware constraint is itself part of the methodology: it forces algorithmic discipline (flat arrays, O(N) updates, sandbox-kill before launch) and rules out brute-force compute as a research strategy. The asymmetry between an autodidact with a Mac M2 and a Microsoft Research or DeepMind team — where the latter does not publicly traverse certain mathematical territory for reputational reasons (the rare-paths doctrine §-3.5) — is the operational vein the project was built to exploit.

---

## 9. Reproducing the canonical record from this repository

```bash
# 1. Compile the canonical engine
g++ -O3 -march=native -std=c++17 -funroll-loops \
    -o TRINCANERO TRINCANEROELDELOSCOJONESPUROS.cpp

# 2. Run the engine (writes its output dump in CWD)
./TRINCANERO

# 3. Verify the output is byte-exact ZE99 1154
python3 verify_ze99_1154.py <output_dump>
```

Expected wall-clock on Mac M2 single-thread at 25% CPU: ~19 seconds for step 2, under one second for step 3. The verification script reports `*** ALL INVARIANTS PASS ***` on success.

The pre-built dump `ze99_1154_numeric.txt` is shipped with the repository for verification without compiling.

---

## 10. References to upstream work

- **Henry Cohn, MIT spherical-codes archive**: [hdl.handle.net/1721.1/153312](https://hdl.handle.net/1721.1/153312). The canonical 1154-vector configuration is reproduced byte-exact against this primary source.
- **Zinoviev, V. A.; Ericson, T.** (1999). New lower bounds for contact numbers in small dimensions. *Problems of Information Transmission*, 35(4), 287–294.
- **De Laat, D.; Leijenhorst, N.** (2024). Solving clustered low-rank semidefinite programs arising from polynomial optimization. *Mathematical Programming Computation* / arXiv:2202.12077 (quadruple-precision SDP achieving K(13) ≤ 2064).
- **Cohn, H.; Elkies, N.** (2003). New upper bounds on sphere packings I. *Annals of Mathematics*, 157(2), 689–714. The linear programming bound LUNA reproduced independently as K(13) ≤ 2233.6.
- **Conway, J. H.; Sloane, N. J. A.** (1999). *Sphere Packings, Lattices and Groups* (3rd ed.). Springer-Verlag. The canonical reference for Λₙ constructions used by Construction A and BW₁₆.
- **Brouwer, A. E.; Etzion, T.** (2011). Some new distance-4 constant weight codes. *Advances in Mathematics of Communications*, 5(3), 417–424. A(13,4,4) = 65 used in the pool generation.
- **Hanani, H.** (1975). Truncated finite planes. *Proc. Sympos. Pure Math.* SQS existence theorem for Steiner system constructions.
- **Cohn, H.; Li, S.** (2024). Sphere packings and kissing numbers in dimensions 17–21. arXiv:2411.04916. The most recent record advances in higher dimensions, by the method LUNA's rare-paths doctrine is designed to operate alongside.

The full citation list, with BibTeX entries, is in `CITATION.md`.
