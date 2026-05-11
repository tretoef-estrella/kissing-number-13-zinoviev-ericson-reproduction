# Main Scientific Contributions

This document distills the five scientific contributions of Project LUNA's attack on K(13) ≥ 1155 into citable form. For full proofs, sandbox transcripts, and external references, see `PAPER_TERMINAL.md` and `CLOSURE_DIM13.md`. For the operational arsenal that produced these results, see `COJONES_SABIOS_TERMINAL.md`.

The world record K(13) = 1154 (Zinoviev–Ericson 1999) was not beaten. These contributions characterize *why* the natural algebraic landscape saturates at 1154, identify the directions where the record may still fall, and release a reproducible methodology for autodidact-driven mathematical research with AI partners.

---

## Contribution 1 — First independent byte-exact reproduction of the ZE99 1154-vector configuration from cold initialization

Engine `TRINCANEROELDELOSCOJONESPUROS` reaches the full 1154-vector Zinoviev–Ericson configuration in approximately 19 seconds on a Mac M2 single-thread at 25% CPU, starting from a 1106-vector anchor derived from the laminated lattice Λ₁₃ floor (Leech–Sloane 1971). The pipeline is the deterministic 24-step Move R-axial→irrational chain: starting from the 1106-anchor plus the 24 pure axials ±4·eᵢ, the engine deterministically removes one axial and adds two specific irrational vectors of the form (0,...,0,±2√3,0,...,0,±2) per gate, for 24 gates exactly, yielding 1106 − 24 + 48 = 1130 + 24 = 1154. The 288 "diamond" vectors with coordinates in (±1)¹² ⊕ (±2) appear as a consequence of the same construction. Verification is independent: a Python kernel (`verify_ze99_1154.py`) confirms the dump against all five invariants of the original construction (squared norm 16, kissing condition saturated at IP = 8 with zero violations, exactly 59,640 tight pairs, exactly 577 antipodal pairs, twelve-valued discrete inner-product spectrum).

**Significance**: prior to this work the Zinoviev–Ericson configuration was an existence proof; no independent reproduction from cold initialization had been published in 27 years. The reproduction makes the construction operationally accessible to any researcher with a laptop and validates the structural picture (F62 profile decomposition) at quantitative depth.

**Reference**: `PAPER_TERMINAL.md` §42; this repository's `TRINCANEROELDELOSCOJONESPUROS.cpp` and `ze99_1154_numeric.txt`.

---

## Contribution 2 — The F58 universal-sink law for natural algebraic perturbations of ℝ¹³ kissing configurations

Across five non-overlapping algebraic families swept during the project — Eisenstein superset constructions (F50), Barnes–Wall BW₁₆ cross-section to ℝ¹³ (F53), exotic √k composite alphabets (F70), composite ℚ+ℚ√3+ℚ√k families (F73), and Eisenstein ℤ[ζ₃] independent constructions via Minkowski embedding with coord-13 integer (F-frente-B-3, V14) — every candidate vector that conflicts with ZE99 clusters its conflicts onto the 48-vector F40 axial-irrational layer rather than onto the bulk 1106 anchor or the 288 diamond layer. The Eisenstein closure (V14, 56.8 million candidates swept in 232 seconds) is the fifth and structurally orthogonal confirmation: among the 82 ZE99 vectors evicted by the Move-D-zeta cascade, **all 48 axial-irrationals of the F40 layer were evicted**, while the 288 diamonds were entirely untouched. This raises F58 from a hypothesis (introduced in `PAPER_TERMINAL.md` line 3603) to a paper-grade empirical structural law: the 48 F40 axial-irrationals form a universal conflict sink under natural algebraic perturbation in ℝ¹³.

**Significance**: F58 explains 27 years of stability of K(13) = 1154 under attack from natural algebraic constructions. Any future attempt to break ZE99 from a natural algebraic alphabet must either explicitly address F58 (find a construction whose conflicts route around the F40 layer) or pivot to an alphabet algebraically incompatible with the F40 sink — the operational content of the rare-paths doctrine (Contribution 4).

**Reference**: `CLOSURE_DIM13.md` §13.13.3 (F-frente-B-3 statement); `PAPER_TERMINAL.md` line 3603 (F58 origin).

---

## Contribution 3 — The 2²⁶⁴ Steiner-equivalent enjambre and the perfect-matching rigid skeleton of ZE99

The Zinoviev–Ericson configuration of size 1154 is not unique. There exist approximately 3 × 10⁷⁹ (precisely 2²⁶⁴) Steiner-equivalent configurations of size 1154 obtained by replacing any subset of 264 of the 288 diamond vectors with their Steiner cset=1 partners. The Steiner construction itself uses the C(12, 6) = 924 sign-block partitions of coordinates 0–11 with coord-12 magnitude ±2; the IP-filter against bulk_1104 (= 1154 minus 48 F40 minus 2 pure axials) yields exactly 528 candidates with cset = 1 against the full ZE99, paired 2-to-1 with the 264 "Steiner-touchable" diamonds. The remaining **24 "Steiner-untouchable" diamonds** carry as their coord-0..11 support a perfect matching of 6 pairs on K₁₂:

  **{(0, 3), (1, 2), (4, 5), (6, 10), (7, 8), (9, 11)}**

This 6-pair perfect matching is the combinatorial-geometric rigid skeleton of the diamond layer. Together with the V14.alpha-bis quantitative characterization of F40 dense geometric coupling (each F40 vector has degree-2 kissing contact but 302 near-IP partners in the bulk at 0 < IP < 8) and the 100% literal frequency of the ±2 entry in coord-12 across all 288 diamonds, this produces a four-level characterization of ZE99's internal structure that constrains the space of possible 1155-or-larger continuations.

**Significance**: ZE99 is structurally degenerate (2²⁶⁴ equivalent configurations) but rigidly anchored (24 untouchable diamonds with a specific perfect-matching support). Any rare-path attack on K(13) ≥ 1155 that depends on coord-12 privilegio is constrained by this rigid skeleton; any attack that depends on F40 layer modification is constrained by the F58 universal-sink law (Contribution 2).

**Reference**: `CLOSURE_DIM13.md` §13.14.3 (F-frente-B-6 statement); `PAPER_TERMINAL.md` §42 (F62 profile decomposition origin).

---

## Contribution 4 — The rare-paths doctrine: an operational filter for future record attempts

After empirically sweeping ~7.13 × 10⁹ candidates across all naturally available algebraic alphabets (real-quadratic ℚ+ℚ√3 norm² = 24, integer native shell multi-swap delta ∈ {1, 2, 3, 4}, Eisenstein ℤ[ζ₃] Minkowski-embedded, Barnes–Wall BW₁₆ cross-section, exotic √k composites) and observing universal saturation at the F58 sink, Project LUNA codifies the **rare-paths doctrine** (`PAPER_TERMINAL.md` §14, `CLOSURE_DIM13.md` §14.6): if a path to K(13) ≥ 1155 existed in conventional mathematics, Cohn–Li 2024, AlphaEvolve 2025, or DeepMind would have found it. The record's 27-year stability empirically argues that the genuine open vein lives in mathematics the academic mainstream **knows** but does not publicly traverse for dim 13 specifically because crossing it costs reputational capital.

The doctrine operationalizes "rare" through a **four-condition filter** that any proposed frente must satisfy:

1. **Outside the F-MASTER swept algebraic alphabet** (real-quadratic, integer multi-swap, Eisenstein, BW₁₆, exotic √k, composite ℚ+ℚ√3+ℚ√k, Eisenstein-via-Minkowski).
2. **Anchored to a named field or construction admitting exact arithmetic** — "cyclotomic generally" or "moduli space loosely" are inadmissible.
3. **Known to the mainstream mathematical literature but not yet published for dim 13 kissing**.
4. **Verifiable bridge to ZE99 in exact arithmetic** — inner products against ZE99 must be computable in some named ring, sandbox-killable before any heavy compute.

Four veins pass the filter and are catalogued in `ANCIENT_MATH_RESEARCH_BRIDGE.md`: **ℚ(ζ₁₀) decagonal cyclotomic** (Vein 1, with golden ratio Φ = (1+√5)/2 entering arithmetic, algebraically orthogonal to ℚ(√3)), **Steiner S(5,6,12) + Mathieu M₁₂ doubly-transitive sporadic action** (Vein 4, identified in V14 as the cleanest sporadic-group passing example), **rigidity-coupled deformation of the 48 axial-irrationals** (mod-p rank test of the contact-graph rigidity matrix as sandbox-kill), and **I Ching D₂₄ hexagram-derived combinatorial structures** (extension of an ILP-optimal D₁₂ found earlier). Two veins are rejected as decorative exotism: Sulba Sutras continued fractions (reduces to ℚ+ℚ√3 in disguise) and Pappus–Desargues projective incidence (fails condition 4).

**Significance**: the doctrine converts an empirical observation (27 years of stability) into an actionable operational filter. It identifies where the record may still be broken and explicitly discards research directions that look exotic but reduce to already-swept algebra. Aggregated calibrated probability of K(13) ≥ 1155 across the four passing rare-path veins is 4–9% in a 12–24 month horizon (post-V15 calibration).

**Reference**: `PAPER_TERMINAL.md` §14; `CLOSURE_DIM13.md` §14.6; `ANCIENT_MATH_RESEARCH_BRIDGE.md`.

---

## Contribution 5 — The dual Constructor/Auditor methodology with both-hats discipline for single-agent fallback

Project LUNA operates under a **dual-Claude architecture**: two instances of the same large language model (Anthropic Claude Opus 4.7) take separated roles. The **Constructor** writes C++ engines, executes Python sandbox computations, and proposes pivots. The **Auditor** verifies mathematics from first principles, ratifies plans before any Mac launch, catches numerical and methodological errors, and explicitly does not issue partial ratification (V14 codification: Auditor's hat is total clean or total dirty, never "clean in essence"). A human **Architect** arbitrates between them and decides engine naming, which follows a deliberately irreverent convention where contractual names are earned by records and never assigned speculatively (directive D26).

When the Auditor is unavailable mid-session (rate limits, context exhaustion, or other operational constraints), the project codifies **both-hats discipline** (V15 §-3.6): the Constructor produces every operational claim twice — once in proposing mode and once in challenging mode — and explicitly attends to five trampa-suspect categories where the discipline is mandatory rather than optional. These categories are: (a) interpreting graph properties as geometric properties, (b) constructing constraint matrices where a constraint type might be missing, (c) testing intuition-driven hypotheses whose negative results may still teach something, (d) extrapolating from sample to population, and (e) reading numerical coincidences as structural. During a single-Claude session on 08 May 2026, three traps were self-caught and graveyarded (G33: topological-vs-geometric reading of F40 contact degree; G34: rigidity matrix without norm-preservation; G35: horizon-hole hypothesis on pure axials) before any contamination reached the Architect.

**Significance**: this is a reproducible operational pattern for AI-partnered mathematical research that does not require human-level mathematical expertise from the human partner. The Architect of Project LUNA is a psychologist by formal training, an autodidact in coding theory and combinatorial optimization, with no professional mathematics affiliation. The dual-Claude architecture made it operationally feasible for a single autodidact with a Mac M2 to sweep ~7.13 × 10⁹ algebraic candidates against the world record K(13) = 1154 under disciplined sandbox-kill protocols, publish thirteen paper-grade structural findings, and codify the rare-paths doctrine in roughly two weeks of focused work.

**Reference**: `LUNA_CONTEXT_BRUTAL.md` (operational constitution); `METHODOLOGY.md` (technical primer); `COJONES_SABIOS_TERMINAL.md` (operational lessons).

---

## What is *not* claimed

This repository **does not** claim K(13) ≥ 1155. The world record at project closure remained K(13) = 1154 (Zinoviev–Ericson 1999). The empirical result of LUNA's attack across roughly 7.13 × 10⁹ candidates was zero records beating 1154. The contributions above are structural understanding and methodology, not new lower bounds.

The four rare-path veins (ℚ(ζ₁₀), Steiner+M₁₂, rigidity-coupled deformation, I Ching D₂₄) are documented as **open research directions**, not as work in progress. Project LUNA closed without opening them; any continuation is by independent researchers under their own banner. The reserved engine name `DECAGONOROMANTICOPERSAREYDESHIELOPOLLITOSRAFA` for the ℚ(ζ₁₀) attack is released along with the rest of the project material; if someone uses it, that is their decision.

The aggregated calibrated probability of K(13) ≥ 1155 across the four open rare-path veins in a 12–24 month horizon (4–9%) is a calibrated estimate honed against the empirical trajectory of the project, not a marketing claim.
