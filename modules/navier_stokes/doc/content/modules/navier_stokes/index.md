# Navier-Stokes Module

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using either the continuous Galerkin finite element
(CGFE) or finite volume (FV) methods. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible formulation (assuming a
constant fluid density), or a density-based, compressible formulation, although
there are plans to add a finite volume weakly-compressible pressured-based implementation in
the not-too-distant future.

For documentation specific to finite element or finite volume implementations,
please refer to the below pages:

- [Continuous Galerkin Finite Element](navier_stokes/cgfe.md)
- [Incompressible Finite Volume](insfv.md)
- [Weakly Compressible Finite Volume](wcnsfv.md)
- [Porous media Incompressible Finite Volume](pinsfv.md)
- [Compressible Finite Volume HLLC](CNSFVHLLCBase.md)
- [Porous media Compressible Finite Volume Kurganov-Tadmor](PCNSFVKT.md)
- [Porous media Compressible Finite Volume HLLC](PCNSFVHLLC.md)
- [Turbulence Modeling Theory](navier_stokes/rans_theory.md)

Here we give a brief tabular summary of the Navier-Stokes implementations:

!table id=navier_stokes_summary caption=Summary of Navier-Stokes implementations
| prefix     | Jacobian   | compressibility      | turbulence support | friction support  | method | advection strategy                |
| ------     | --------   | -------------------- | ------------------ | ----------------  | ------ | ------------------                |
| INS        | Hand-coded | incompressible       | None               | Not porous        | CGFE   | SUPG                              |
| INSAD      | AD         | incompressible       | Smagorinsky        | Not porous        | CGFE   | SUPG                              |
| NS         | Hand-coded | compressible         | None               | Not porous        | CGFE   | SUPG                              |
| INSFV      | AD         | incompressible       | mixing length      | Not porous        | FV     | RC, CD velocity; limited advected |
| WCNSFV     | AD         | weakly compressible  | mixing length      | Not porous        | FV     | RC, CD velocity; limited advected |
| PINSFV     | AD         | incompressible       | None               | Darcy, Forcheimer | FV     | RC, CD velocity; limited advected |
| CNSFVHLLC  | AD         | compressible         | None               | Not porous        | FV     | HLLC, piecewise constant data     |
| PCNSFVHLLC | AD         | compressible         | None               | Darcy, Forcheimer | FV     | HLLC, piecewise constant data     |
| PCNSFVKT   | AD         | compressible         | None               | Darcy, Forcheimer | FV     | Kurganov-Tadmor, limited data     |

Table definitions:

- AD: automatic differentiation
- SUPG: Streamline-Upwind Petrov-Galerkin
- RC: Rhie-Chow interpolation
- CD: central differencing interpolation; equivalent to average interpolation
- HLLC: Harten Lax van Leer Contact
- data: includes both the advector, velocity, and the advected quantities
- limited: different limiters can be applied when interpolating cell-centered
  data to faces. A summary of limiter options can be found in
  [Limiters/index.md]

For an introductory slideshow on the use of the Navier Stokes Finite Volume solvers in MOOSE, we refer the visitor to the [Navier Stokes Workshop Slides](slides/index.md optional=True).

!syntax complete groups=NavierStokesApp
