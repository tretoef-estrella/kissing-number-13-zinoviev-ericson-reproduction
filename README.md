# Kissing Number K(13): An Independent Reproduction and Structural Analysis

> First independent byte-exact reproduction of the Zinoviev–Ericson (1999) configuration of **1154 unit spheres simultaneously tangent to a central unit sphere in ℝ¹³**, with the full optimization pipeline, 13 paper-grade structural findings, the rare-paths doctrine, and the dual Constructor/Auditor methodology with both-hats discipline released as a complete, reproducible artifact.

**Status**: Closed (May 2026). This repository documents the completed attack by Project LUNA on the open question whether K(13) ≥ 1155. The world record **K(13) = 1154** (Zinoviev–Ericson 1999, 27 years standing at project closure) was **not** beaten. The natural algebraic landscape was empirically exhausted across roughly 7.13 billion candidates. The record remains open; this repository documents what was tried, what closed negative, what was learned, and the four rare-path veins identified as the doctrine-conformant directions for any continuation.

---

## What this repository contributes

1. **The first independent byte-exact reproduction of the ZE99 1154-vector configuration from cold initialization**, reached in ~19 seconds on a Mac M2 single-thread (25% CPU) by engine `TRINCANEROELDELOSCOJONESPUROS` via a 24-step deterministic Move R-axial→irrational chain. Full verification scripts included.

2. **Thirteen paper-grade structural findings on the saturation of K(13) = 1154 under algebraic perturbation**, including:
   - **F58 universal-sink law**: any algebraically natural perturbation in ℝ¹³ clusters its conflicts onto the 48-vector F40 axial-irrational layer of ZE99. Confirmed independently by five non-overlapping algebraic families (Eisenstein superset, BW₁₆ cross-section, exotic √k, composite ℚ+ℚ√3+ℚ√k, and Eisenstein ℤ[ζ₃] via Minkowski embedding).
   - **The 2²⁶⁴ Steiner-equivalent enjambre**: ZE99 is not the unique K(13)=1154 configuration. Roughly 3·10⁷⁹ Steiner-equivalent configurations exist under a coord-12-preserving replacement operation, leaving 24 "Steiner-untouchable" diamonds whose support is a perfect 6-pair matching on coordinates 0–11.
   - **The cset-parity structural barrier**: empty intervals in the conflict-set spectrum appear consistently across real-quadratic, integer, and imaginary-quadratic alphabets — elevating parity from shell-specific observation to a structural feature of the dim-13 algebraic landscape.

3. **The rare-paths doctrine**: a four-condition operational filter for any future record attempt, codified from 27 years of stability of K(13) ≥ 1154 under institutional attack. Three rare-path veins (ℚ(ζ₁₀) decagonal, Steiner+M₁₂ sporadic, rigidity-coupled 48-irrationals deformation) are identified as the directions where the record, if it falls, is most likely to fall.

4. **The dual Constructor/Auditor methodology with both-hats discipline**: a reproducible operational pattern for autodidact-driven mathematical research using two AI instances with separated roles, with a fallback discipline (`§-3.6 both-hats`) for single-agent sessions including a graveyard catalogue of three self-caught traps as canonical examples.

---

## Verify the record reproduction in under five minutes

```bash
git clone https://github.com/REPLACE_WITH_YOUR_USERNAME/kissing-number-13-zinoviev-ericson-reproduction.git
cd kissing-number-13-zinoviev-ericson-reproduction
python3 verify_ze99_1154.py ze99_1154_numeric.txt
```

Expected output ends with `*** ALL INVARIANTS PASS ***`. The script checks: 1154 vectors, all at squared norm 16, zero pairs with inner product exceeding 8, exactly 59,640 tight pairs at IP=8, exactly 577 antipodal pairs at IP=-16, exactly twelve distinct inner-product values, and zero duplicates.

To compile and run the engine that produced the dump from cold initialization:

```bash
g++ -O3 -march=native -std=c++17 -funroll-loops \
    -o TRINCANERO TRINCANEROELDELOSCOJONESPUROS.cpp
./TRINCANERO              # writes ze99_1154_numeric.txt in ~19 s on Mac M2
```

---

## What is in this repository

**Canonical paper and operational distillation**

- [`PAPER_TERMINAL.md`](PAPER_TERMINAL.md) — full paper, 4848 lines, all findings F1–F73 and F-frente-A/B-1..6 with proofs, sandbox transcripts, and external citations.
- [`CLOSURE_DIM13.md`](CLOSURE_DIM13.md) — formal closure document for dim 13, primary reference for external citation.
- [`MAIN_DISCOVERIES.md`](MAIN_DISCOVERIES.md) — five citable scientific contributions, concise.
- [`COJONES_SABIOS_TERMINAL.md`](COJONES_SABIOS_TERMINAL.md) — operational arsenal and lessons distilled for any researcher who wants to continue.
- [`METHODOLOGY.md`](METHODOLOGY.md) — standalone technical primer on the pipeline, the D190 quadruple-verify protocol, and the F40 deterministic chain.
- [`GUIDE_FOR_EVERYONE.md`](GUIDE_FOR_EVERYONE.md) — plain-language tour, no math background required.
- [`NEW_DISCOVERIES.md`](NEW_DISCOVERIES.md) — empty journal template for any reader who builds on this work.

**Canonical data and verifiers**

- [`ZE99_1154_DATA.md`](ZE99_1154_DATA.md) — provenance, parser, and verification invariants for the 1154-vector configuration.
- [`ze99_1154_numeric.txt`](ze99_1154_numeric.txt) — the 1154 vectors as numeric float CSV, produced by `TRINCANEROELDELOSCOJONESPUROS` cold-start.
- [`verify_ze99_1154.py`](verify_ze99_1154.py) — independent Python kernel that verifies the record byte-exact.
- [`d190_paso4_verify.py`](d190_paso4_verify.py) — D190 protocol step 4: independent verifier for 1155-candidate engine dumps.

**Engines (C++ single-thread, Mac M2 25% CPU)**

- [`TRINCANEROELDELOSCOJONESPUROS.cpp`](TRINCANEROELDELOSCOJONESPUROS.cpp) — canonical engine; reproduces ZE99 1154 in ~19 s via 24-step deterministic Move R-axial→irrational chain.
- [`TRINCANERO_v1.cpp`](TRINCANERO_v1.cpp) — predecessor, ceiling at 986 (graveyard G23–G25, lesson preserved in `COJONES_SABIOS_TERMINAL.md`).
- [`HORMONEROCHUMINEROPREMIUM_v2_1.cpp`](HORMONEROCHUMINEROPREMIUM_v2_1.cpp) — first LUNA engine, Construction A + layer stacking.
- [`DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS_v3.cpp`](DOSPIRATASDUROSCONTRESCOJONESPUROSENCERADOSYPERFUMADOS_v3.cpp) — 1155-record-attempt engine #1, did not beat 1154.
- [`DOSPIRATASMUYPIRATASSONCOJONESNOBRAVATAS.cpp`](DOSPIRATASMUYPIRATASSONCOJONESNOBRAVATAS.cpp) — 1155-record-attempt engine #2, did not beat 1154.

**Operational context and rare-paths catalog**

- [`LUNA_CONTEXT_BRUTAL.md`](LUNA_CONTEXT_BRUTAL.md) — project constitution, including the rare-paths doctrine codification.
- [`ANCIENT_MATH_RESEARCH_BRIDGE.md`](ANCIENT_MATH_RESEARCH_BRIDGE.md) — five mathematical veins identified as untested for dim 13, with probability calibration.
- [`MATHEMATICAL_COMPANION_ONBOARDING_LUNA22.md`](MATHEMATICAL_COMPANION_ONBOARDING_LUNA22.md) — historical handoff document for a potential dim-22 continuation. **Archival only; not operational.**

**Citation and license**

- [`CITATION.md`](CITATION.md) — BibTeX, APA, IEEE, plus the required upstream citations.
- [`CITATION.cff`](CITATION.cff) — machine-readable for GitHub's "Cite this repository" button.
- [`LICENSE`](LICENSE) — MIT.

---

## Mathematical context, briefly

The **kissing number** K(n) is the maximum number of non-overlapping unit spheres that can simultaneously touch a central unit sphere in ℝⁿ. The problem is open in dimension 13: the best known **lower bound** is **K(13) ≥ 1154** (Zinoviev–Ericson, *IEEE Trans. Inform. Theory*, 1999), the best known **upper bound** is **K(13) ≤ 2064** (De Laat–Leijenhorst, 2024, via quadruple-precision semidefinite programming on the Cohn–Elkies linear programming bound).

The lower bound has stood for 27 years. The natural algebraic constructions (laminated lattice Λ₁₃ = 906, Construction A binary codes, BW₁₆ cross-section to ℝ¹³, Eisenstein superset constructions in dimensions divisible by 6 followed by cross-section to 13) all saturate well below 1154. Zinoviev and Ericson reached 1154 by adding a layer of 48 axial-irrational vectors with coordinates in ℤ[√3]/2 to a Λ₁₃-derived 1106-vector anchor, plus 288 "diamond" vectors with coordinates in (±1)¹² ⊕ (±2). The construction is structured, finite, and reproducible; this repository reproduces it byte-exact from a 1106-vector cold start.

The empirical question this repository investigates is whether the same kind of algebraic perturbation that takes the laminated lattice from 906 to 1154 — Move R-axial→irrational and its successors — can be pushed further to 1155 or beyond. The empirical answer, after sweeping ~7.13 × 10⁹ candidates across all naturally available algebraic alphabets and their compositions, is: **not within the natural algebraic landscape, and the conflict structure (F58 universal-sink) is now characterized at quantitative depth**. The record, if it falls, falls on a rare path.

---

## Methodology in one paragraph

Two AI instances (Anthropic Claude Opus 4.7) operated under separated roles: a **Constructor** that proposed engines and sandbox computations, and an **Auditor** that verified mathematics from first principles and vetoed Mac launches that did not pass sandbox-kill discipline (directive D24). A human **Architect** (Rafael Amichis Luengo, Madrid) arbitrated, decided engine names according to a deliberately irreverent naming convention (directive D26: contractual names are earned by records, never assigned speculatively), and held the human clock. When the Auditor was unavailable mid-session, the Constructor applied **both-hats discipline** (§-3.6): every operational claim is produced twice, once in proposing mode and once in challenging mode, with explicit attention to five trampa-suspect categories. Three traps were self-caught during a single-Claude session (G33, G34, G35) before any contamination of project documents reached the Architect. Every record claim, had one occurred, would have been validated by the **D190 quadruple-verify protocol**: in-engine FINAL_VERIFY, roundtrip serialization, structural sanity vs ZE99 and the De Laat–Leijenhorst bound, and an independent Python kernel verifier (`d190_paso4_verify.py`).

For full detail, read `METHODOLOGY.md`.

---

## Citation

If you use this work, please cite it together with the original Zinoviev–Ericson 1999 record and the De Laat–Leijenhorst 2024 upper bound. See `CITATION.md` for ready-to-paste BibTeX and APA forms.

---

## License

MIT. See `LICENSE`. The 1154-vector configuration data itself is derived from Henry Cohn's MIT spherical-codes archive ([hdl.handle.net/1721.1/153312](https://hdl.handle.net/1721.1/153312)) and verified byte-exact against that primary source; the original ZE99 construction is due to Zinoviev and Ericson.

---

## Operational motto

> *Aquí se baten récords mundiales. No se viene a jugar. Calidad relojero suizo. Pereza prohibida. Vivir para ver. Y ahora también: los caminos raros son el camino.*

The record was not beaten. The arsenal, the findings, the doctrine, and the discipline are released so that whoever picks up the chase from a rare-path direction starts ahead of where this project started.

**Architect**: Rafael Amichis Luengo. Madrid, Spain. May 2026.
