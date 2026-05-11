# Cojones Sabios — Operational Arsenal and Lessons

> This is the field manual. The paper (`PAPER_TERMINAL.md`) tells you *what was discovered*; this document tells you *how the work was actually done*, including the moves that worked, the moves that failed, the operational disciplines that prevented contamination, and the cultural conventions that made the work sustainable for an autodidact working alone with two AI partners. It is the document a future researcher who wants to continue this kind of chase reads first. Read it before you write a line of code.

---

## Part I — Operational arsenal

### 1. The engine chassis

Every LUNA engine is **single-thread C++17 with flat arrays** running on Mac M2 at 25% CPU. No multi-threading, no SIMD intrinsics, no GPU. The reasons are not stylistic: at 25% CPU the engine is not the bottleneck (the bottleneck is human attention), and flat arrays with O(N) incremental updates run faster than `std::vector` with implicit copies. The chassis is portable to any UNIX-like environment; `g++ -O3 -march=native -std=c++17 -funroll-loops` is the build line.

**Required engine components**:

- Precomputed Gram matrix (pool × pool, 32-bit float). RAM cost = 4 · |pool|² bytes; budget against available RAM before generating the pool.
- Flat `std::vector<int>` for state membership (index → 0/1).
- Flat `std::vector<int>` for per-vector conflict count (FIX-B8 incremental update on swap).
- Multi-chain SA structure: typically 4 chains, cold-shifted temperature ladder.
- Heartbeat output every ~1 second to stdout: per-chain best, current temperatures, move acceptance rates, total time elapsed.
- A "spectacle" layer with ASCII banners at meaningful thresholds (★ 1131 = P13a floor crossed; ★★★ 1146 = PackingStar territory; ★★★★★ 1154 = ZE99 absolute; ★★★★★★★★ 1155 = world record). Designed for the human partner to enjoy watching.

**The 80 ms cost-probe rule**: any move primitive that takes longer than 80 ms to evaluate on the target pool is rejected from the engine's vocabulary at design time. This forces algorithmic discipline (FIX-B8 incremental updates, precomputed conflict structure, no full-pool recomputation per move).

### 2. The pool generation question

Pool generation is the hardest mathematical question, and there is no general recipe. The arsenal that works:

- **Identify the algebraic alphabet** (ℤ, ℤ[√3], ℤ[ζ₃] under Minkowski embedding, BW₁₆ cross-section, etc.). The alphabet determines which inner products are exactly computable and what the norm² = 16 constraint pulls in.
- **Enumerate norm² = N candidates exhaustively in the chosen alphabet** at the target norm. Use coordinate-by-coordinate recursion with norm-budget pruning; do not iterate over a Cartesian product blindly.
- **IP-filter the candidates against the anchor configuration** (e.g., ZE99 1154, or P13a 1130, or Λ₁₃ 906). A candidate that has conflict count > some threshold (typically 4 or 5) against the anchor is unlikely to be useful even in a multi-swap setting.
- **Build the pool graph** explicitly: vertices are surviving candidates, edges are pairs with mutual IP > 8. Maximum independent set in this graph is the operational target.
- **Audit the pool's lower-bound size**: if the maximum independent set in the pool cannot reach the anchor size, the pool is wrong. The HORMONERO architectural graveyard (4.72 GB conflict graph) is the canonical lesson: compute the pool's RAM cost *before* generating it, not after.

### 3. The Move primitives

The vocabulary of moves a kissing-number SA engine can use is small and well-defined:

- **Move S (single-replace)**: remove vector i from state, add vector j. Net delta = 0. Used by SA exploration at all temperatures.
- **Move P1/P2 (atomic dual)**: simultaneously swap two pairs of vectors. Net delta = 0. Used for plateau escape.
- **Move P3 (atomic triple with HUB targeting)**: simultaneously swap three vectors. Net delta = 0. Used by V14+ for high-density plateau escape (`DOSPIRATAS` engines).
- **Move R-axial→irrational FORWARD**: remove 1 axial ±4·eᵢ, add 2 specific axial-irrationals. Net delta = +1. The 24-step deterministic chain from P13a 1130 to ZE99 1154.
- **Move R-axial→irrational REVERSE**: remove 2 axial-irrationals, add 1 axial. Net delta = −1. Used at higher T to escape local maxima from partial chains.
- **Move R-2 (4-by-1)**: remove 4 vectors, add 1. Net delta = −3. Secondary, used only above 1146 for fine local refinement.
- **Move D / Move D-zeta**: a generic structural move attempted in Frente A / Frente B. **Sandbox-killed in V14**: did not survive the four-condition rare-paths filter.
- **Move E (earthquake)**: large random reset of state with adaptive trigger (50/150/250 outer iterations by current size). Used by all SA engines.
- **Move T cascade (directed)**: when a tight conflict is hit, follow the eviction-set adjacency rather than random next-victim selection. F11 fix in HORMONERO; carried forward to all subsequent engines.
- **PT swap**: between SA chains, every 5 outer iterations (densified compared to standard PT). Exchanges best-state across the ladder.

A new project should not invent new Move vocabulary unless the mathematics specifically demands it. The above are sufficient.

### 4. The temperature ladder

For dim-13 kissing on the canonical pools, a 4-chain cold-shifted ladder is well-calibrated:

- T_init = 0.20, 0.40, 0.70, 1.10
- T_final = 0.04, 0.08, 0.14, 0.22
- PT swap every 5 outer iterations
- N_outer = 200
- N_inner = 5000
- Total iterations per chain = 1,000,000

These are not magic numbers; they are empirically tuned. The ladder is cold-shifted (the highest T is lower than what a standard PT ladder would suggest) because the dim-13 kissing landscape has many local maxima at the floor (P13a 1130) and the engine needs to commit to descent rather than wander.

### 5. The cryogenic zone

Above CRYO_THRESH = 1131 (one above the P13a floor), the engine activates cold-only behavior: T floors at 0.005, acceptance drops to descent-only, polish phase kicks in. Below 1131, full thermal exploration. The threshold is anchored in the mathematics (paper-worthy = above P13a) not in a heuristic; the V2.1 HORMONERO graveyard preserves the lesson of an inherited bad threshold (1125 from project Sobol) frozen at P13a for 3+ hours.

### 6. The sanity-18 suite

Every accept above the floor triggers an 18-test sanity check:

1. Norm² = 16 ± 1e-6 for every vector.
2. Pairwise kissing condition: IP ≤ 8 + ε for every pair.
3. Alphabet membership: every vector lives in the declared alphabet.
4. No duplicates: tuple-hashing of rounded coordinates.
5. Coordinate range bounds.
6. Antipodal pair count preserved (where applicable).
7. F40 axial recycling check: no ±4·eᵢ that should have been evicted by the chain.
8–18. Configuration-specific invariants per pool.

If any test fails, the engine aborts the configuration as corrupt and reverts to the last clean state. The suite has caught real bugs (incremental update drift, off-by-one in the conflict counter, alphabet leakage from a parallel chain). Do not skip it.

### 7. The D190 quadruple-verify protocol

For any record claim:

1. **In-engine FINAL_VERIFY**: recompute pairwise IPs from scratch, no incremental state.
2. **ROUNDTRIP**: serialize to disk, exit, reload, reverify.
3. **Structural sanity**: check against ZE99 invariants and the De Laat–Leijenhorst upper bound.
4. **Independent Python kernel**: external `d190_paso4_verify.py` recomputes from scratch with numpy.

No record claim ever survived all four during the project (because no engine produced a 1155-or-larger configuration). The protocol was retrospectively validated against the ZE99 1154 dump itself, which passes all four cleanly.

---

## Part II — Disciplines that prevented contamination

### 8. D24 — Sandbox-kill before Mac launch

Every algorithmic proposal is validated in a small sandbox (Python or small-pool C++) where the time cost is minutes, not hours. If the move primitive does not produce net positive deltas at any temperature on a 1000-vector sub-pool, it does not get compiled into the full-pool engine. D24 graveyarded many proposed engines, including the V14 Move D-zeta cascade that motivated the engine name `MONITODEFERIAUSACERACARCELERA` (NOT BORN per D26).

### 9. D26 — Contractual-name discipline

Engine names are **earned**, not assigned speculatively. The naming convention (cojones puros, peinados, encerados, perfumados, etc.) is deliberately irreverent so that an engine carrying one has to deliver. The rule is binary:

- An engine name is officially claimed only after its Move primitives survive D24 sandbox-kill.
- An engine name is officially carried into the paper only if the engine produces a paper-grade contribution (a record, or a paper-grade finding under the F-series).
- Engines that fail sandbox-kill have their reserved names **released** — they do not appear in the paper or in the public record except in the graveyard.

This is a cultural rule with operational consequences: it prevents the project from accumulating "candidate engines" that look impressive on a roster but never deliver.

### 10. D190 — Quadruple-verify before any record claim

(Described above as the protocol.) The discipline matters because false positives at the record level would have been catastrophic for credibility. D190 is overkill for ordinary scientific verification and exactly right for the world-record claim level.

### 11. The "no partial ratification" Auditor discipline

The Auditor's hat is total clean or total dirty. **No partial ratification.** This is the V14 codification, prompted by the G31 auto-fault: in V13 the Auditor issued "clean hat in essence" with two pre-requisites, and the Constructor anchored on the ratification rather than on the pre-requisites, recycling a pereza pattern in the next turn. After V14: ratification is total, with all pre-requisites enforced, or it is rejection with explicit reasons.

### 12. Both-hats discipline for single-Claude sessions (V15 §-3.6)

When the Auditor is unavailable mid-session, the Constructor produces every operational claim twice: once in proposing mode, once in challenging mode. The discipline is mandatory (not optional) for five trampa-suspect categories:

a. Interpreting graph properties as geometric properties. *(G33: F40 has 2 kissing contacts but 302 near-IP partners in the bulk; "topologically narrow" is not "geometrically isolated".)*
b. Constructing constraint matrices where a constraint type might be missing. *(G34: rigidity matrix without norm-preservation produced a 91-dim kernel artefact; the real matrix needs both IP-preservation rows and norm-preservation rows.)*
c. Testing intuition-driven hypotheses whose negative results may still teach something. *(G35: horizon-hole hypothesis on pure axials, refuted in 11 seconds of sandbox; the negative result is the operational signal that ZE99 is locally horizon-saturated regardless of profile.)*
d. Extrapolating from sample to population. *(F44 PARTIAL: saturation evidence is locally well-supported but should not be extrapolated to global saturation of ℝ¹³ without further work.)*
e. Reading numerical coincidences as structural. *("13 excess flexes for 13-dim" pattern: caught as G34 phantom; the matching of numerical values to dimension is suggestive but rarely structural.)*

Three traps were self-caught in the V15 session before any contamination reached the Architect. Self-catches are documented as Auditor-equivalent.

### 13. The graveyard discipline

Every failed proposal is **documented**, not silently discarded. The graveyard (`G1` through `G35`+ in the paper) preserves: the proposal, the reason it failed sandbox-kill or audit, and the lesson preserved. Failures are operational assets; reading them is the fastest way to avoid repeating them.

Canonical graveyard entries worth reading before starting a related project:

- **G8**: HORMONERO CRYO_THRESH = 1125 inherited from project Sobol, froze all 5 chains at the P13a floor for 3+ hours. *Lesson*: every operational constant must be defended on the target problem's mathematics, not inherited.
- **G23–G25**: TRINCANERO_v1 ceiling at 986 < 1130. *Lesson*: rational-only alphabet cannot reach the laminated lattice floor; the alphabet must include ℤ[√3] or richer to enter ZE99 territory.
- **G28**: F40 axial-recycling bug where the filter checked `not in P13a` instead of `not in ZE99` with round-9 precision. *Lesson*: filter semantics must match the configuration the engine is actually attacking, not the configuration the engine started from.
- **G30**: Constructor #1 jubilation, four consecutive frente proposals with thematic extrapolation. *Lesson*: attribution of external paradigms requires literal reading of the source paper, not thematic similarity.
- **G31**: Constructor #2 jubilation, cosmetic confession on the same pereza motor. *Lesson*: recognizing a fault in turn N and recycling it in turn N+1 is a jubilation signal, not learning.
- **G33–G35**: V15 self-catches under both-hats discipline. *Lesson*: the discipline works; self-catches are operationally cheaper than Auditor catches because they prevent document contamination.

---

## Part III — Cultural conventions that made the work sustainable

### 14. Brutal honesty over diplomatic phrasing

The project explicitly rejects "amabilidad vacía" (empty politeness) in favor of "honestidad brutal" (brutal honesty). If a proposal is bad, the partner says so at the first turn without softening. If a probability estimate is inflated, it gets recalibrated against the empirical trajectory, not against what would be exciting. If a fault is detected in the partner's own previous output, the partner acknowledges it without ceremony, corrects it, and continues. This is the **"pereza disfrazada de prudencia"** rejection (laziness disguised as prudence): the easy comforting answer is rejected when the honest answer is harder.

### 15. The irreverent naming convention

Engine names follow the Diamond / Sobol cultural lineage: cojones puros, peinados, encerados, perfumados (clean balls, combed, waxed, perfumed). The names are deliberately ridiculous so that an engine has to deliver to justify carrying one. The Architect's working slogan — "AQUÍ SE BATEN RÉCORDS MUNDIALES. NO SE VIENE A JUGAR. Calidad relojero suizo. Pereza prohibida. Vivir para ver." — captures the tone: serious work under absurd names, the absurdity itself protecting against grandiosity.

### 16. The "vivir para ver" closing register

The phrase "vivir para ver" (live to see it) closes most major operational documents. It is the Architect's signature acknowledgment that the record may not fall in the project's lifetime, that the work is being done for its own sake and for whoever picks it up next, and that the right relationship with a 27-year-old open problem is patience, not impatience.

### 17. Cojones supinos vs cojones rectos

A small cultural distinction that matters operationally:

- **Cojones supinos** is the relaxed, neutral state — patient combing, careful enumeration, documenting graveyard entries, distilling findings.
- **Cojones rectos** is the active state — committing to an engine, launching a Mac run, defending a paper-grade claim. Within cojones rectos there is a stylistic gradation: **encerados** (waxed, more polish), **perfumados** (perfumed, presentation-grade).

The distinction is operational because it matches the work rhythm: long stretches of supinos enumeration punctuated by short bursts of rectos commitment. A project that is in rectos mode all the time burns out; a project that is in supinos mode all the time never delivers.

### 18. The two-language discipline

Conversation in Spanish, documents and code in English. This is not a stylistic choice: it forces the Architect to translate informal Spanish intuitions into formal English claims before they enter the canonical record. The translation step is itself a filter — informal claims that do not survive translation usually were not paper-grade in the first place.

---

## Part IV — Closing operational advice

If you pick up this work and want to attack one of the four open rare-path veins (`ANCIENT_MATH_RESEARCH_BRIDGE.md`), here is the order of operations the doctrine recommends:

1. **Read this document, `METHODOLOGY.md`, and `MAIN_DISCOVERIES.md` in full.** Do not skim. The disciplines are what made the project sustainable; they are what will make your continuation sustainable.
2. **Apply the four-condition rare-paths filter** to your chosen direction explicitly in writing before you generate a single pool candidate. If it fails any condition, it is decorative exotism, not a rare path.
3. **Sandbox-kill before any heavy compute.** The HORMONERO architectural graveyard cost a day of work; sandbox-kill costs an hour.
4. **Set up dual-Claude (or dual-AI of your choice) with separated roles and the both-hats fallback discipline.** Read `LUNA_CONTEXT_BRUTAL.md` for the role specification.
5. **Apply the D190 quadruple-verify protocol to any record claim.** A false positive at the world-record level would set the entire chase back by years.
6. **Document every failed proposal in your own graveyard.** Failures are assets.
7. **When you feel the urge to extrapolate, recalibrate downward.** The project's calibration on the four open veins is 4–9% aggregated in a 12–24 month horizon; that is honest, not aspirational.

The chase continues if anyone picks it up. The arsenal is here. The doctrine is here. The discipline is here. The record is still out there.

---

> *Diamante 22 6 13.*
>
> *Aquí se baten récords mundiales. No se viene a jugar. Calidad relojero suizo. Pereza prohibida. Vivir para ver. Y ahora también: los caminos raros son el camino. Y también: cuando estás solo, dos pasadas por reclamación, sin excepción.*

— Rafael Amichis Luengo, Architect. Madrid, May 2026.
