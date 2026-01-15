# Navier-Stokes Module

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using either the continuous Galerkin finite element
(CGFE) or finite volume (FV) methods. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible or weakly-compressible formulation (assuming a
constant or pressure-independent fluid density), or a density-based, compressible formulation.

For documentation specific to finite element or finite volume implementations,
please refer to the below pages:

- [Incompressible Finite Volume](insfv.md)
- [Weakly Compressible Finite Volume](wcnsfv.md)
- [Weakly compressible finite volume using a linear discretization and a segregated solvealgorithm (SIMPLE/PIMPLE)](linear_wcnsfv.md)
- [Porous media Incompressible Finite Volume](pinsfv.md)
- [Continuous Galerkin Finite Element](navier_stokes/cgfe.md)
- [Hybridized Discontinous Galerkin (HDG) Finite Element](inshdg.md)
- [Hybrid Continuous/Discontinuous Galerkin Finite Element](navier_stokes/hcgdgfe.md)
- [Compressible Finite Volume HLLC](CNSFVHLLCBase.md)
- [Porous media Compressible Finite Volume Kurganov-Tadmor](PCNSFVKT.md)
- [Porous media Compressible Finite Volume HLLC](PCNSFVHLLC.md)
- [Turbulence Modeling Theory](navier_stokes/rans_theory.md)

Here we give a brief tabular summary of the Navier-Stokes implementations:

!table id=navier_stokes_summary caption=Summary of Navier-Stokes implementations
| prefix     | Solve method |Jacobian   | compressibility                | turbulence support          | friction support  | discretiz. | advection strategy      |
| ------     | -----------  | --------- | ------------------------------ | --------------------------- | ----------------  | ------ | --------------------------------- |
| INS        | Newton/PJFNK | Hand-coded | incompressible                | None                        | Not porous        | CGFE   | SUPG                              |
| INSAD      | Newton/PJFNK | AD         | incompressible                | Smagorinsky                 | Not porous        | CGFE   | SUPG                              |
| INSFE      | Newton/PJFNK | Hand-coded | incompressible                | mixing length               | Not porous        | CGFE   | SUPG                              |
| PINSFE     | Newton/PJFNK | Hand-coded | incompressible                | mixing length               | porous            | CGFE   | SUPG                              |
| NS         | Newton/PJFNK | Hand-coded | compressible                  | None                        | Not porous        | CGFE   | SUPG                              |
| INSChorin  | Newton/PJFNK | Hand-coded | incompressible                | None                        | Not porous        | CGFE   | Chorin predictor-corrector        |
| INSFV      | Newton/PJFNK | AD         | incompressible                | mixing length; $k-\epsilon$ | Not porous        | FV     | RC, CD velocity; limited advected |
| WCNSFV     | Newton/PJFNK | AD         | weakly compressible           | mixing length               | Not porous        | FV     | RC, CD velocity; limited advected |
| Linear(WCNS)FV | SIMPLE   | N/A        | weakly compressible           | $k-\epsilon$                | Not porous        | FV     | RC velocity; limited advected   |
| WCNSFV2P   | Newton/PJFNK | AD         | weakly compressible; 2-phase  | mixing length               | Not porous        | FV     | RC, CD velocity; limited advected |
| LinearWCNSFV2P | SIMPLE   | N/A        | weakly compressible; 2-phase  | None                        | Not porous        | FV     | RC velocity; limited advected   |
| PINSFV     | Newton/PJFNK | AD         | incompressible                | mixing length               | Darcy, Forcheimer | FV     | RC, CD velocity; limited advected |
| CNSFVHLLC  | Newton/PJFNK | AD         | compressible                  | None                        | Not porous        | FV     | HLLC, piecewise constant data     |
| PCNSFVHLLC | Newton/PJFNK | AD         | compressible                  | None                        | Darcy, Forcheimer | FV     | HLLC, piecewise constant data     |
| PCNSFVKT   | Newton/PJFNK | AD         | compressible                  | None                        | Darcy, Forcheimer | FV     | Kurganov-Tadmor, limited data     |

Table definitions:

- INS: incompressible Navier-Stokes
- AD: automatic differentiation
- WCNS: weakly-compressible Navier-Stokes
- WCNS2P: weakly-compressible Navier-Stokes 2-phase
- CNS: compressible Navier-Stokes
- PINS or PCNS: porous incompressible Navier-Stokes or porous compressible Navier-Stokes
- LinearFV: the [linear finite volume discretization](linear_fv_design.md)
- SUPG: Streamline-Upwind Petrov-Galerkin
- RC: Rhie-Chow interpolation
- CD: central differencing interpolation; equivalent to average interpolation
- HLLC: Harten Lax van Leer Contact
- data: includes both the advector, velocity, and the advected quantities
- limited: different limiters can be applied when interpolating cell-centered
  data to faces. A summary of limiter options can be found in
  [Limiters/index.md]

Note that the INS and INSFE kernel sets are redundant in terms of targeted
functionality. Historically, the INS kernel set was developed in this module and
the INSFE kernel set was developed in the SAM application (where there it was
prefixed with "MD") [!citep](hu2021sam). With
the Nuclear Energy Advanced Modeling and Simulation (NEAMS) program dedicated
to consolidating fluid dynamics modeling, SAM capabilities are being migrated
upstream as appropriate into the common module layer. In the not-too-distant
future these kernel sets will be consolidated into a single set.

For an introductory slideshow on the use of the Navier Stokes Finite Volume solvers in MOOSE, we refer the visitor to the [Navier Stokes Workshop Slides](modules/navier_stokes/intro/index.md optional=True).

As Navier-Stokes Finite Volume solvers continue to evolve in MOOSE, many new solvers have been added to the Navier-Stokes module. The following table provides a summary of the readiness of different solvers and the capabilities they support. Since many of these solvers are still under active development, feel free to reach out to the MOOSE team for updates on the latest progress or follow our monthly newsletter updates.

!table id=navier_stokes_solver_summary caption=Summary of Navier-Stokes Solver Capabilities
| Capability         | model                     | FE Newton  | FV Newton                                                    | FV Nonlinear SIMPLE                                          | FV Linear SIMPLE       |
| ------------------ | ------------------------- | ---------- | -----------------------------------------------------------  | ------------------------------------------------------------ | ---------------------- |
| Transient          |       --                  | Yes        | Yes                                                          | Yes                                                           | Yes                                                          |
| Turbulence         | Mixing length             | Yes        | Yes                                                          | Yes                                                          |                        |
|                    | $k-\epsilon$              |            | Yes                                                          | Yes                                                          | Yes                    |
|                    | $k-\omega$ SST            |            |                                                              | in [PR #28151](https://github.com/idaholab/moose/pull/28151) |                        |
| Two-phase          | Mixture model             | Yes        | Yes                                                          | Yes                                                          | Yes |
|                    | Eulerian-Eulerian         |            |                                                              | Yes                                                          |                        |
| Porous Flow        |       --                  | Yes        | Yes                                                          | Yes                                                          |                        |
| Compressibility    | Incompressible            | Yes        | Yes                                                          | Yes                                                          | Yes                    |
|                    | Weakly compressible       |            | Yes                                                          | Yes                                                          | Yes                    |
|                    | Compressible              |            | Yes                                                          |                                                              |                        |
| Coupling           | Domain overlapping (SAM)  |            | Yes                                                          | Yes                                                          |                        |
|                    | Domain decomposition (THM)     |       | in [PR #28528](https://github.com/idaholab/moose/pull/28528) |                                                              |                        |
|                    | Domain decomp. MultiApp w/ SCM |       | Yes                                                          |                                                              |                        |
| Scalar Transport   |       --                  |            | Yes                                                          | Yes                                                          | Yes                    |
| Physics Syntax     | Flow                      |            | Yes                                                          |                                                              | Yes                    |
|                    | Fluid heat transfer       |            | Yes                                                          |                                                              | Yes                    |
|                    | Solid phase heat transfer |            | Yes                                                          |                                                              |                        |
|                    | Two phase                 |            | Yes                                                          |                                                              | Yes |
|                    | Turbulence                |            | Yes                                                          |                                                              |      Yes             |
|                    | Scalar transport          |            | Yes                                                          |                                                              | Yes                    |


!syntax complete groups=NavierStokesApp
