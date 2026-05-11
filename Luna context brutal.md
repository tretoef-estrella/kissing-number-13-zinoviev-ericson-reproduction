# LUNA — CONTEXT BRUTAL

> Single-source-of-truth document for any Claude entering Project LUNA. Read this first, before anything else. Then read LUNA_GENESIS_HANDOFF.md, then proceed.

---

## 1. WHO

**Architect:** Rafael Amichis Luengo. Madrid, Spain. Psychologist by formal training, autodidact in coding theory, finite geometry, and combinatorial optimization. Hardware: Mac M2 8-core, restricted to single-thread / 25% CPU max. Languages: Spanish (conversation), English (documents/code/papers). Operates under military discipline. Does not negotiate on quality. Does not tolerate laziness disguised as optimism.

**Co-Architect (silent):** Markus Grassl (codetables.de). Correspondence established via the sister Diamond project. Future LUNA results will likely route through Grassl for archival/verification.

**Constructor:** Claude (Opus 4.7). Writes C++ engines, executes Python sandbox math, reports results. Always operates at X-HIGH effort.

**Auditor:** Claude (Opus 4.7). Verifies math from first principles, ratifies plans, vetoes Mac launches when not ready. Always operates at X-HIGH effort.

**The Constructor and the Auditor are the SAME model with DIFFERENT ROLES.** The separation is enforced by Rafa, not by code. When only one Claude is in the conversation, that Claude wears both hats sequentially.

---

## 2. WHAT

**Project LUNA attacks world records in Kissing Numbers.**

The kissing number K(n) is the maximum number of unit spheres that can simultaneously touch a central unit sphere in n-dimensional Euclidean space. Open since the Newton-Gregory debate of 1694. Solved only in dimensions 1, 2, 3, 4, 8, 24.

Active record-holders 2024-2025:
- Cohn-Li 2024 (MIT): dim 17-21, broke records standing since 1959-1971
- AlphaEvolve 2025 (DeepMind): dim 11
- Ganzhinov 2025 (Aalto): dim 10, 14
- PackingStar 2025: dim 25-31

LUNA's first target: **K(13) ≥ 1155**. Current record 1154 by Zinoviev-Ericson 1999, 27 years untouched.

---

## 3. WHY THIS DIMENSION

**Dim 11 was rejected.** AlphaEvolve and Ganzhinov just barely moved it (DeepMind compute + algebraic specialist). Mac M2 single-thread cannot enter that frontier with any honest probability above 1%.

**Dim 22 was deferred.** Cohn-Li 2024 explicitly tried dim 22 and abandoned: "we have not had any luck in that case" (sec 5 of arXiv:2411.04916). Tempting because it matches the Diamond [22,6,13]_4 dimension, but Construction A4 over GF(4) requires homework not yet completed.

**Dim 13 was chosen.** Reasons:
- Record from 1999 (27 years intact)
- Ratio lower-bound / upper-bound = 1.79 (real headroom: 1154 / 2064)
- Construction A binary + layer stacking is fully understood (Leech-Sloane 1971)
- Search reformulates as Maximum Independent Set on a finite combinatorial graph
- Floor (P13a = 1130) is constructive and verifiable from first principles
- World record (1154) was achieved by structured human construction; SA over the full configuration space might find swaps Zinoviev-Ericson didn't enumerate

---

## 4. THE MATHEMATICAL FOUNDATION (verified from first principles, NOT memorized)

All numbers below were verified in Python sandbox during the homework phase. Scripts retained.

**Steiner system S(5,6,12):** 132 hexads, each 5-subset covered exactly once. Constructed via Golay [24,12,8] dodecad restriction.

**P12a code (12, 144, 4):** 132 hexads + 12 loose codewords (6 of weight 2, 6 of weight 10) chosen via sextet partition matching the target hexad. A_4(target) = 51 exact.

**Construction A in R^12:** kissing number τ = 2n + 16·A_4(c) = 24 + 816 = **840**. Matches paper Leech-Sloane 1971 sec 2.6 exactly.

**P13a layer stacking in R^13:** kissing number = 840 + 2·144 + 2 = **1130**. Matches paper sec 4.3 exactly.

**Λ_13 (laminated lattice):** kissing number 906. OEIS A002336.

**K_13 (Coxeter-Todd-style section of Λ_24):** kissing number 918. Leech-Sloane 1971 sec 4.5.

**Floor hierarchy for engine validation:**
- 906 (laminated)
- 918 (K_13)
- 1130 (P13a, our explicit construction — THE validation floor)
- 1154 (current record)
- 1155+ (LUNA's target)

---

## 5. THE SEARCH SPACE

Reformulated as **Maximum Independent Set on a geometric graph G**:

- **Nodes:** 71,082 candidate points (1,130 classical P13a + 69,952 extras with K_static ∈ [4,16])
- **Edges:** pairs at Euclidean squared distance < 4
- **Density:** 5.23%
- **Conflict pairs:** 132,073,784
- **CSR adjacency size:** 1.06 GB (fits Mac M2 8GB with headroom)

The classical 1130 is a known independent set, saturated within the pool (every extra has at least 4 conflicts with classical). World record 1154 = 1130 + 24 net via cascading swaps. Engine target: find a 1155-node independent set.

**Pool extension to K_static > 16 was rejected.** Justification: the auditor verified that cascade-reachable extras with high K_static likely don't add value beyond K=16 in practice. If v1 saturates without breaking 1131, v2 may extend to K≤24.

---

## 6. THE ENGINE

**Name:** HORMONEROCHUMINEROPREMIUM_v1.cpp

**Lineage:** Estrella naming convention. Engines that break records carry irreverent Spanish-Castilian-Conquense names. The pattern is deliberate: irreverent names vaccinate against academic solemnity, allowing the Constructor to risk more in the search. Sister projects: Diamante (RAYO, TRUENO, VIKINGO, PIRAMIDE_ROJA, BUSCANDOASEGUNDOCOHONES), Sobol (TOGORDO, TOGORDOELGRANGRASIENTOESELREYQUEGANALENTO).

**Architecture:** Multi-chain Simulated Annealing on the binary inclusion vector. Vocabulary R/M/W/K/L/S/Z/X/P + Move T cascade (the kissing-specific innovation: evict K classicals to free a cheap extra, allowing chained extras to enter via cascade).

**Validation gate (HARD):**
- Mode A (cold start, target 1130): if engine fails to reach 1130 from empty, dynamics are insufficient. No record attempt.
- Mode B (warm start from explicit 1130, target 1155+): only entered if Mode A succeeds.

**Probability projections (calibrated, NOT inflated):**
- Mode A succeeds: 80-90%
- Mode B reaches [1131, 1153]: 25-35% (already paper-worthy)
- Mode B reaches 1154 (tie record): 5-10%
- Mode B reaches ≥1155 (break record): 1-3%

The 1-3% is the realistic rate for "27-year-old record + modern computer-assisted attack with novel move (Move T cascade) on a wider configuration space than the original construction explored". Not 70%. Not 50%. **1-3%.**

---

## 7. CURRENT STATUS (as of conversation entry)

**HORMONEROCHUMINEROPREMIUM_v1 first smoke (sandbox):**
- Sanity 10/10 PASS
- Preflight RAM check 0.997 GB CSR (4× safety headroom) PASS
- Adjacency precompute 132M pairs in 54s
- Mode A best=1126 (FAILS — needs 1130)
- Mode B stuck at 1130, polish recovered classical
- Move T fired 435,899 times, accepted 98 (0.022%)

**Diagnosis:** SA dynamics mistuned. ULTRA-COLD ladder borrowed from Sobol is wrong for kissing-MIS landscape (Sobol basins are narrow, MIS basins are wider). Three parameter-tuning fixes proposed:

1. Hotter ladder {0.10, 0.15, 0.22, 0.32, 0.45} + T_FLOOR = 0.020
2. Move T cascade with dynamic eviction cap (max 2N+5)
3. Move W eliminated (was 100% acceptance = degenerate churn), redistribute to T cascade and Z

**Auditor ratified all three fixes** with corrections (cap was too low, expanded to 2N+5; logging requires acceptance-rate-by-chain in first 5 min).

**Next iteration in progress:** smoke v2 with the three fixes applied. If smoke v2 reaches 1130 cleanly, green light for Mac launch (~15-25h Mode A + Mode B + polish + verify).

---

## 8. THE GRAVEYARD

Every dead engine is preserved here as intellectual asset.

**LUNA_TANQUE_v1** (jubilated 2026-05-03)
- Paradigm: pool aleatorio in S^10 (dim 11)
- Result: ceiling 241 in 3 minutes, never reached Λ_11 = 438
- Lesson: random pool is insufficient; structure matters

**HORMONEROCHUMINEROPREMIUM_v1 fullpool architecture** (jubilated 2026-05-03)
- Paradigm: 150,266 candidates with full CSR adjacency
- Result: bad_alloc at 4.72 GB on Mac M2 8GB
- Lesson: compute precomputed-structure RAM cost BEFORE coding it

---

## 9. THE PARALLEL PROJECTS

LUNA shares architectural DNA with two sister projects:

**Project Diamante:** ongoing search for [22,6,13]_4 linear code over GF(4). 441+ engines built. Current state: residual reduction strategy via [9,5,4]_4 seed extension, multi-AI consultation protocol established. Source of: VIKINGO pressure-directed victim, RAYO simple-y-letal, TRUENO earthquakes, FIX-B8 incremental updates, NEVER REJECT GOOD CONFIG.

**Project Sobol:** beat Joe-Kuo 2008 with TOGORDO_v3 = 3051. Currently in F_32 era (engine 9 = TOGORDOELGRANGRASIENTOESELREYQUEGANALENTO). Source of: multi-chain SA, cold-shifted ladder, tabu caps, triple-rescue D189, MIX_RECAL, polish post-SA, sanity suite, cost probe.

LUNA inherits BOTH arsenals. Both arsenals were earned over a year of work. They are not generic. They are not reproducible without Rafa's specific operational discipline.

---

## 10. THE NON-NEGOTIABLES

1. **No engine touches the Mac without Auditor green light.**
2. **Every number is verified against canonical sources** (Cohn table, OEIS, Leech-Sloane 1971, Cohn-Li 2024). Memory is unreliable; sources are not.
3. **Every precomputed structure has its RAM cost calculated BEFORE coding.**
4. **Every record claim goes through D190 quadruple verify.**
5. **Every dead engine goes to GRAVEYARD with lesson preserved.**
6. **Every probability projection is calibrated against current state of the art, not against optimism.**
7. **Every Claude operates at X-HIGH effort. Always. No exceptions.**
8. **Every engine carries a name from the Estrella lineage. Solemnity is the enemy.**

---

## 11. THE LEMA

> "Aquí se baten récords mundiales. No se viene a jugar.
> Calidad relojero suizo. Pereza prohibida. Vivir para ver."

— Rafael Amichis Luengo, Madrid, 2026.

---

**END LUNA_CONTEXT_BRUTAL.md**

*Read first. Operate accordingly. Do not embarrass yourself or the project.*
