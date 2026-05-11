# A Guide for Everyone

> This is a plain-language tour of what Project LUNA tried to do, what it actually accomplished, and what it did not. No mathematics background is required. The technical primer is in `METHODOLOGY.md`; the formal paper is in `PAPER_TERMINAL.md`; this guide is for the curious reader.

---

## 1. The thirteen-dimensional ball-packing problem

Imagine a billiard ball, perfectly round, lying on a table. Around it, how many other billiard balls of the same size can you arrange so that each one is touching the central one — actually pressed against it, kissing it — without any two of the surrounding balls overlapping each other? In two dimensions (coins on a table), the answer is six. You can see it: hexagonal packing, six pennies around a central penny. In three dimensions, the answer is twelve: stack oranges in a crate, and twelve of them can touch a central orange. This number — six for the plane, twelve for ordinary space — is called the **kissing number**.

The kissing number changes as you go to higher dimensions. In four-dimensional space the answer is exactly 24. In eight dimensions it is exactly 240. In twenty-four dimensions it is exactly 196,560. These exact answers come from beautiful structures that mathematicians have discovered over centuries — special lattices, sphere packings derived from error-correcting codes, configurations related to fundamental constants of the universe (the Leech lattice in dimension 24 is genuinely connected to deep physics).

But there is a dimension where the answer is not known. In **dimension 13**, the best known answer is that you can fit at least **1154** spheres kissing the central one, but the absolute maximum could be as high as **2064**. The lower number (1154) was discovered by two Russian mathematicians, V. A. Zinoviev and T. Ericson, in 1999. Their construction has stood for 27 years without anyone finding a configuration with even one more sphere. Project LUNA's central question was: can we find a configuration with 1155 or more, and beat the world record?

---

## 2. Why does this matter outside mathematics?

Sphere packing in high dimensions is not a recreational puzzle. The same mathematics shows up in:

- **Error-correcting codes**: the way satellites send images of distant planets back to Earth without losing the data depends on dense packings of vectors in spaces of hundreds or thousands of dimensions. The better the packing, the better the code, the cleaner the image.
- **Cryptography**: post-quantum cryptography (the cryptography designed to resist attacks from future quantum computers) is built on the difficulty of finding shortest vectors in high-dimensional lattices. Packing density is a direct dual of that difficulty.
- **Wireless communication and signal processing**: every modern wireless protocol uses high-dimensional vector packings to multiplex signals.
- **Machine learning and high-dimensional statistics**: the geometry of high-dimensional space is full of counterintuitive phenomena that depend on how densely things can be packed — concentration of measure, curse of dimensionality, the structure of generalization.

Improving the lower bound K(13) ≥ 1154 to K(13) ≥ 1155 would not, by itself, change any of these applications. But the methods that produce such an improvement might. The Zinoviev–Ericson 1999 construction itself spawned a small literature of related techniques used elsewhere. And the methods this project codified — the rare-paths doctrine, the dual-Constructor/Auditor methodology, the both-hats discipline — are reusable for many open problems beyond this one.

---

## 3. Who built this and how

The project was built by **Rafael Amichis Luengo**, a psychologist by formal training who taught himself coding theory, combinatorial optimization, and the mathematics of sphere packing over years of self-study. He worked from a single Mac M2 laptop, single-threaded at 25% CPU, in Madrid, Spain. The compute he used would have been laughably small at any large university or technology company. What he had instead was discipline, patience, and a working partnership with two AI instances of Anthropic's Claude Opus 4.7 model — one playing a **Constructor** role (writing C++ code, running computations) and the other playing an **Auditor** role (checking mathematics from first principles, rejecting proposals that did not survive scrutiny). The architect — Rafael — arbitrated between them and made the final calls.

This methodology — one human, two AI roles, sandbox-kill discipline before any heavy compute, brutal honesty over diplomatic phrasing, and graveyard documentation of every failed proposal — is itself one of the project's main contributions. It is documented in detail in `METHODOLOGY.md` and `LUNA_CONTEXT_BRUTAL.md`.

---

## 4. What was attempted and what happened

The natural way to attack K(13) ≥ 1155 is to take Zinoviev and Ericson's 1154-vector configuration and try to add a 1155th vector that does not conflict with any of the first 1154. If no such vector exists, you try to swap one vector out and put two different vectors in, increasing the size by one. If that does not work, you try larger combinatorial moves. And if simple combinatorial moves do not work, you try generating candidate vectors from richer algebraic alphabets — extensions of the rational numbers by square roots, cube roots, roots of unity, and so on.

Project LUNA tried, systematically, **all of these**. The chase swept roughly **7.13 billion candidate vectors** drawn from every naturally available algebraic family: ordinary integers, rationals extended by √3, the Eisenstein integers ℤ[ζ₃], a cross-section of the Barnes–Wall lattice BW₁₆ from dimension 16 down to dimension 13, composite alphabets mixing several square roots, and others. **In every single case, the result was: zero records.** Not one configuration of size 1155 was found, in any of these alphabets, under any of the search strategies tried.

What the project did discover — and this is the substance of its scientific contribution — is *why* the natural algebraic alphabets all saturate at 1154. Every attempt to add a vector clusters its conflicts onto a specific 48-vector layer inside the Zinoviev–Ericson configuration (the so-called F40 axial-irrational layer, vectors with coordinates involving √3). This pattern — christened the **F58 universal-sink law** — was confirmed by five different non-overlapping algebraic families. It is now a paper-grade empirical law of the dim-13 algebraic landscape.

Two further structural discoveries followed. First, the Zinoviev–Ericson configuration of size 1154 is **not unique**: it is one of approximately 3 × 10⁷⁹ equivalent configurations of the same size (the "Steiner-equivalent enjambre"), all related by replacing certain "diamond" vectors with their Steiner-system partners. Second, inside the 288 diamond vectors there is a rigid skeleton of 24 vectors that **cannot** be replaced by this operation; their coordinate support forms a specific perfect matching on twelve points, listed precisely in the paper. The combination characterizes ZE99's internal structure at quantitative depth and constrains what any future 1155-or-larger configuration must respect.

---

## 5. The "rare paths" doctrine

After 7.13 billion candidates returned zero records, the project asked itself: if the record is breakable, why has nobody broken it in 27 years? Cohn–Li at MIT broke the records in dimensions 17 to 21 in 2024. DeepMind's AlphaEvolve broke the record in dimension 11 in 2025. PackingStar broke records in dimensions 25 to 31 in 2025. **Dimension 13 was specifically left alone.** Why?

The project's answer — the **rare-paths doctrine** — is that conventional mathematics has been exhausted on this dimension, and the open vein lives in territory the academic mainstream knows but does not publicly traverse for reputational reasons. Mixing sphere packing with sporadic finite groups (Mathieu's group M₁₂ acting on a Steiner system), with cyclotomic fields outside the standard list (the decagonal field ℚ(ζ₁₀) involving the golden ratio), with continuous moduli of rigidity-coupled deformation, or with combinatorial designs from outside standard pedagogy — these are all directions that a Microsoft Research or Stanford principal investigator would *notice* but would not write a paper on, because the audience for "dim 13 kissing record via sporadic groups" is small enough that the reputational return is not worth the time. An autodidact with a Mac M2 has no such reputational cost; this asymmetry is the genuine open vein.

The doctrine was codified into a four-condition filter. Any proposed direction must (1) live outside the algebraic territory already exhausted, (2) name its mathematical home concretely, (3) be known to mainstream mathematics but not applied to dim 13, and (4) admit exact arithmetic verification against the Zinoviev–Ericson configuration. Four directions pass the filter: **ℚ(ζ₁₀) decagonal cyclotomic**, **Steiner system S(5,6,12) with Mathieu M₁₂ action**, **rigidity-coupled deformation of the 48 axial-irrationals**, and **I Ching D₂₄ hexagram-derived combinatorial structures**. Each is documented in `ANCIENT_MATH_RESEARCH_BRIDGE.md` with a calibrated probability estimate. None was opened before project closure; the doctrine is left as a roadmap for anyone who picks up the chase.

---

## 6. What this repository is

This repository is the **complete release** of everything Project LUNA produced. It includes:

- The full mathematical paper (`PAPER_TERMINAL.md`) and the formal closure document for dim 13 (`CLOSURE_DIM13.md`).
- The canonical engine C++ source code, and the byte-exact 1154-vector configuration it produces (`ze99_1154_numeric.txt`).
- An independent Python verifier that confirms the configuration is a valid kissing configuration (`verify_ze99_1154.py`) — runs in under a second.
- The four other engines that were tried and failed to beat 1154, with their lessons preserved.
- The operational documents: the project constitution (`LUNA_CONTEXT_BRUTAL.md`), the rare-paths catalog (`ANCIENT_MATH_RESEARCH_BRIDGE.md`), the operational arsenal and lessons (`COJONES_SABIOS_TERMINAL.md`).
- A handoff document for a potential continuation in dimension 22, archived (not operational).

If you are a mathematician or a curious technically-trained reader: the paper and the methodology document have the math. If you are a software engineer or a researcher who wants to reuse the AI-partnered methodology: the operational documents have the discipline. If you just want to *see* the world record configuration: run the engine, or look at the dump, or run the verifier.

---

## 7. What this repository is *not*

This repository **does not** claim that K(13) ≥ 1155. The world record at project closure was, and remains, K(13) = 1154. The project did not beat it.

The four open rare-path directions are documented as **open problems**, not as work in progress. Project LUNA closed without opening them. Any continuation is by independent researchers under their own banner — the project's name (LUNA), its engine naming conventions, and its operational discipline are released as cultural artifacts, not as obligations for any future work.

The calibrated estimate that the record can be broken in the next 12 to 24 months along one of the four rare-path directions is in the range of 4 to 9 percent — a real but small probability. It is honest, not aspirational.

---

## 8. The closing motto

> *Aquí se baten récords mundiales. No se viene a jugar. Calidad relojero suizo. Pereza prohibida. Vivir para ver.*
>
> *(Here world records are broken. We did not come to play. Swiss-watchmaker quality. Laziness forbidden. Live to see it.)*

The record was not broken. The arsenal, the findings, the doctrine, and the discipline are released so that whoever picks up the chase from a rare-path direction starts further along than this project did.

**Rafael Amichis Luengo**, Madrid, May 2026.
