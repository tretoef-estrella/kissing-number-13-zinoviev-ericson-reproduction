# Citation

If you use this repository — code, data, methodology, or findings — please cite the project together with the upstream works it builds on. The independent contribution of this repository is the byte-exact reproduction of the Zinoviev–Ericson 1999 configuration, the structural findings F-frente-A/B-1..6 on dim-13 saturation, the rare-paths doctrine, and the dual Constructor/Auditor methodology with both-hats discipline. The world-record kissing number K(13) ≥ 1154 is due to Zinoviev and Ericson and should always be cited alongside.

---

## How to cite this repository

### BibTeX

```bibtex
@misc{amichisluengo2026_kissing13,
  author       = {Amichis Luengo, Rafael},
  title        = {Kissing Number $K(13)$: An Independent Reproduction
                  and Structural Analysis. Project LUNA},
  year         = {2026},
  month        = may,
  howpublished = {GitHub repository},
  url          = {https://github.com/REPLACE_WITH_YOUR_USERNAME/kissing-number-13-zinoviev-ericson-reproduction},
  note         = {First independent byte-exact reproduction of the
                  Zinoviev--Ericson 1999 K(13) = 1154 configuration in
                  $\mathbb{R}^{13}$; thirteen paper-grade structural
                  findings on dim-13 saturation; rare-paths doctrine
                  codified}
}
```

### APA

Amichis Luengo, R. (2026). *Kissing Number K(13): An Independent Reproduction and Structural Analysis. Project LUNA* [GitHub repository]. https://github.com/REPLACE_WITH_YOUR_USERNAME/kissing-number-13-zinoviev-ericson-reproduction

### IEEE

[1] R. Amichis Luengo, "Kissing Number K(13): An Independent Reproduction and Structural Analysis. Project LUNA," GitHub repository, May 2026. [Online]. Available: https://github.com/REPLACE_WITH_YOUR_USERNAME/kissing-number-13-zinoviev-ericson-reproduction

### Chicago

Amichis Luengo, Rafael. "Kissing Number K(13): An Independent Reproduction and Structural Analysis. Project LUNA." GitHub repository. May 2026. https://github.com/REPLACE_WITH_YOUR_USERNAME/kissing-number-13-zinoviev-ericson-reproduction.

---

## Required upstream citations

If you cite this repository, please also cite the upstream work it depends on:

### The original Zinoviev–Ericson 1999 record

```bibtex
@article{zinoviev1999_kissing,
  author  = {Zinoviev, V. A. and Ericson, T.},
  title   = {New lower bounds for contact numbers in small dimensions},
  journal = {Problems of Information Transmission},
  year    = {1999},
  volume  = {35},
  number  = {4},
  pages   = {287--294}
}
```

### The De Laat–Leijenhorst 2024 upper bound

```bibtex
@article{delaat2024_kissing,
  author  = {de Laat, David and Leijenhorst, Nando},
  title   = {Solving clustered low-rank semidefinite programs arising
             from polynomial optimization},
  journal = {Mathematical Programming Computation},
  year    = {2024},
  note    = {Achieves $K(13) \leq 2064$ via quadruple-precision SDP on
             the Cohn--Elkies linear programming bound},
  eprint  = {2202.12077},
  archivePrefix = {arXiv}
}
```

### The Cohn–Elkies linear programming bound

```bibtex
@article{cohn2003_lpbounds,
  author  = {Cohn, Henry and Elkies, Noam},
  title   = {New upper bounds on sphere packings I},
  journal = {Annals of Mathematics},
  year    = {2003},
  volume  = {157},
  number  = {2},
  pages   = {689--714}
}
```

### Conway–Sloane canonical reference

```bibtex
@book{conway1999_splag,
  author    = {Conway, John H. and Sloane, Neil J. A.},
  title     = {Sphere Packings, Lattices and Groups},
  edition   = {3},
  publisher = {Springer-Verlag},
  year      = {1999},
  isbn      = {978-0-387-98585-5}
}
```

### Brouwer–Etzion (A(13,4,4) = 65)

```bibtex
@article{brouwer2011_codes,
  author  = {Brouwer, Andries E. and Etzion, Tuvi},
  title   = {Some new distance-4 constant weight codes},
  journal = {Advances in Mathematics of Communications},
  year    = {2011},
  volume  = {5},
  number  = {3},
  pages   = {417--424}
}
```

### Hanani SQS existence

```bibtex
@inproceedings{hanani1975_sqs,
  author    = {Hanani, Haim},
  title     = {Truncated finite planes},
  booktitle = {Proceedings of Symposia in Pure Mathematics},
  year      = {1975}
}
```

### Henry Cohn's spherical-codes data archive

```bibtex
@misc{cohn_kissing_tables,
  author       = {Cohn, Henry},
  title        = {Spherical codes and kissing-number tables},
  howpublished = {MIT mathematics, online archive},
  note         = {File \texttt{dimensions1-24.txt} contains the
                  Zinoviev--Ericson $K(13) = 1154$ configuration
                  reproduced byte-exact in this repository},
  url          = {https://hdl.handle.net/1721.1/153312}
}
```

### Cohn–Li 2024 (records in dims 17–21, context for the rare-paths doctrine)

```bibtex
@misc{cohnli2024_kissing,
  author       = {Cohn, Henry and Li, Shiyu},
  title        = {Sphere packings and kissing numbers in dimensions
                  17 to 21},
  year         = {2024},
  eprint       = {2411.04916},
  archivePrefix = {arXiv}
}
```

---

## Choosing what to cite from this repository

A pragmatic guide:

- **If you cite the 1154 record itself**: cite Zinoviev–Ericson 1999. This repository is a verification artifact, not the original work.
- **If you reproduce the configuration using this repository's code**: cite both Zinoviev–Ericson 1999 (the construction) and this repository (the reproduction software).
- **If you cite the F58 universal-sink law, the 2²⁶⁴ Steiner-equivalent enjambre, or any other F-frente-A/B-1..6 finding**: cite this repository and `PAPER_TERMINAL.md` for the specific section.
- **If you cite the rare-paths doctrine**: cite this repository (the doctrine is original to it).
- **If you cite the dual Constructor/Auditor methodology with both-hats discipline**: cite this repository (the methodology is original to it).
- **If you cite the upper bound K(13) ≤ 2064**: cite De Laat–Leijenhorst 2024. This repository reproduces the Cohn–Elkies linear programming bound K(13) ≤ 2233.6 independently as a consistency check but does not contribute to the upper-bound story.

---

## Project metadata

- **Architect**: Rafael Amichis Luengo (Madrid, Spain).
- **AI partners**: Anthropic Claude Opus 4.7, in dual Constructor/Auditor roles. The contractual contributions of each Claude instance to specific findings are documented in the authorship lists of `PAPER_TERMINAL.md` and `CLOSURE_DIM13.md`.
- **Project name**: LUNA (sister of Estrella / Sobol Campaign and Diamond / coding-theory project).
- **Status at closure**: Project closed May 2026 with the world record K(13) = 1154 not beaten; four rare-path veins documented as open research directions.

This work was conducted independently, without institutional funding, on a Mac M2 laptop at 25% single-thread CPU. There are no conflicts of interest to declare.
