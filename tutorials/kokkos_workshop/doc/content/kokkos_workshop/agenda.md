# Workshop Overview id=kokkos_workshop_agenda

!---

## Agenda

!style! fontsize=75%

| Time | Session | Format |
| :- | :- | :- |
| 0:00 | Welcome | 5 min |
| 0:05 | [Part 1](#kokkos_workshop_intro): Why Kokkos-MOOSE, architecture, and build | Lecture, 15 min |
| 0:20 | [Exercise 1](#kokkos_workshop_ex1): Run and convert an input file | Hands-on, 15 min |
| 0:35 | [Part 2](#kokkos_workshop_model): The programming model | Lecture, 20 min |
| 0:55 | [Exercise 2](#kokkos_workshop_ex2): Spot the bug | Hands-on, 10 min |
| 1:05 | +Break+ | 10 min |
| 1:15 | [Part 3](#kokkos_workshop_kernels): Kernels | Lecture, 20 min |
| 1:35 | [Exercise 3](#kokkos_workshop_ex3): Port a nonlinear diffusion kernel | Hands-on, 25 min |
| 2:00 | [Part 4](#kokkos_workshop_poly): Polymorphism without virtual functions | Lecture, 15 min |
| 2:15 | [Exercise 4](#kokkos_workshop_ex4): A polymorphic boundary condition family | Hands-on, 15 min |
| 2:30 | +Break+ | 10 min |
| 2:40 | [Part 5](#kokkos_workshop_materials): Materials | Lecture, 15 min |
| 2:55 | [Exercise 5](#kokkos_workshop_ex5): Coupled, on-demand, and stateful materials | Hands-on, 20 min |
| 3:15 | [Part 6](#kokkos_workshop_systems): BCs, NodalKernels, AuxKernels, Functions, UserObjects | Lecture, 20 min |
| 3:35 | [Exercise 6](#kokkos_workshop_ex6): An AuxKernel and a reducer postprocessor | Hands-on, 15 min |
| 3:50 | [Part 7](#kokkos_workshop_perf): Performance, debugging, limitations, Q&A | Lecture, 10 min |

!style-end!

!---

## Who This Workshop Is For

- MOOSE application developers who already write `Kernel`, `Material`, `IntegratedBC`, and
  `Postprocessor` objects in C++
- No prior GPU or Kokkos experience is assumed
- By the end you should be able to:

  - Decide whether an object can be ported and what has to change
  - Port kernels, materials, boundary conditions, auxiliary kernels, and postprocessors
  - Build your own polymorphic base classes without virtual functions
  - Recognize the mistakes that compile cleanly but break on the GPU
  - Verify and profile a Kokkos-MOOSE object

!---

## Environment Check

Complete this before the workshop; building MOOSE with Kokkos takes a while.

A Kokkos-enabled MOOSE is required. PETSc built by `scripts/update_and_rebuild_petsc.sh` already
downloads Kokkos and Kokkos Kernels.

- +NVIDIA GPU+ (CUDA): PETSc configured with `--with-cuda --with-cuda-arch=<arch>` (e.g. `80` for
  `sm_80`), then MOOSE configured with `./configure --with-kokkos`
- +No GPU+: `./configure --with-kokkos=cpu` builds the same objects against the Kokkos OpenMP
  backend. All exercises work, and this is the easiest configuration for debugging.

Check that the executable you will use reports the `kokkos` capability:

```bash
$ ./kokkos_training-opt --show-capabilities | grep kokkos
```

!alert note
Intel GPUs (SYCL) and AMD GPUs (HIP) have build support through PETSc, but NVIDIA CUDA is the
most extensively tested GPU backend.

!---

## Exercise Application

The exercises add objects to a small application. Create it once:

```bash
$ cd ~/projects
$ ./moose/scripts/stork.sh KokkosTraining
$ cd kokkos_training
$ make -j 8
```

- Headers go in `include/`, Kokkos sources (`*.K`) go in `src/`; the build picks up `*.K` files
  automatically
- Objects register with the `"KokkosTrainingApp"` label
- Inputs for each exercise live wherever you like, e.g. `kokkos_training/exercises/`

!alert note title=Reference solutions
`tutorials/kokkos_workshop/kokkos_training` in the MOOSE repository is this application with every
exercise solved: the objects in `include/` and `src/`, and an input and regression test per
exercise in `tests/ex*`. Build it with `make` and check it with `./run_tests`.
