# Flat Interface Surface Plasmon Polariton Benchmark

This example case describes the two-dimensional surface plasmon polariton
(SPP) benchmark and verification test for a flat metal--dielectric interface. An SPP is a transverse-magnetic (TM) wave bound to the
interface, oscillating along it and decaying evanescently into both media
[!citep](raether1988). Because the metal permittivity is negative, the squared
wavenumber changes sign across the interface, which makes this case a useful check
of the interface conditions and of the Robin port conditions in [EMRobinBC.md].

## Model Geometry

!style halign=left
In this scenario, the geometry is a metal half-space ($y < 0$) and a dielectric
half-space ($y > 0$) separated by a flat interface at $y = 0$, as shown in
[spp-benchmark-geometry]. An SPP enters at the left boundary ($x = 0$), travels in
the $+x$ direction, and exits at the right boundary ($x = L$). Parameters for this
scenario are shown in [spp-benchmark-parameters]. All lengths are given in an
arbitrary unit $\ell$, and the free-space wavenumber is chosen so that the SPP
wavelength is $\lambda_\mathrm{SPP} = 10\,\ell$.

!media spp_flat_interface_geometry.png
       style=width:100%;margin:auto;
       id=spp-benchmark-geometry
       caption=Two-dimensional metal--dielectric geometry, with an incoming SPP at left and an absorbing boundary at right.

!table id=spp-benchmark-parameters caption=Model parameters for the SPP benchmark study.
| Parameter (unit) | Value(s) |
| - | - |
| Domain length, $L$ ($\ell$) | 20 |
| Domain height ($\ell$) | 40 |
| Metal relative permittivity, $\varepsilon_m$ | $-2$ |
| Dielectric relative permittivity, $\varepsilon_d$ | $1$ |
| Free-space wavenumber, $k_0$ ($\ell^{-1}$) | $0.4443$ |
| SPP wavelength, $\lambda_\mathrm{SPP}$ ($\ell$) | 10 |

## Governing Equations and Boundary Conditions

!style halign=left
In this simulation, both the real and imaginary components of the electric field
wave will be simulated separately. For TM polarization in two dimensions, the
non-zero field components are $E_x$, $E_y$, and $H_z$, and all three follow from
$E_x$, so only the scalar component $E_x = E_\mathrm{real} + iE_\mathrm{imag}$ needs
to be modeled. In each subdomain $j \in \{m, d\}$, it satisfies the scalar Helmholtz
equation

!equation id=spp-helmholtz
\nabla^2 E_x + k_j^2 E_x = 0, \qquad k_j^2 = \varepsilon_j k_0^2

where

- $\varepsilon_j$ is the relative permittivity of subdomain $j$, and
- $k_0 = \omega / c$ is the free-space wavenumber.

This is a standard diffusion-reaction style equation without a source, so the
[Diffusion.md] and [MatReaction.md] objects are used for each real and imaginary
component in each subdomain, with the material properties `kSquared_lower`
($-2k_0^2$) and `kSquared_upper` ($k_0^2$) supplying $k_j^2$. Each component in each
subdomain is a separate first-order Lagrange variable (four in total). The real and imaginary equations are
coupled only through the Robin boundary conditions.

At the interface, the tangential electric field is continuous,

!equation id=spp-continuity
E_x\big|_{m} = E_x\big|_{d}

which is enforced with [MatchedValueBC.md], and continuity of the normal displacement
field gives

!equation id=spp-flux
\frac{1}{\varepsilon_m} \frac{\partial E_x}{\partial n}\bigg|_{m} = \frac{1}{\varepsilon_d} \frac{\partial E_x}{\partial n}\bigg|_{d}

which is enforced with [InterfaceDiffusion.md]. That kernel establishes flux
equivalence $D\,\partial_n E_m = D_\mathrm{neighbor}\,\partial_n E_d$ and does not
enforce continuity of the variable, which is why it is combined with
[MatchedValueBC.md]. To reproduce [spp-flux], $D = 1$ in the metal and
$D_\mathrm{neighbor} = -2$ in the dielectric. These are $\varepsilon^{-1}$ on each
side multiplied by the common factor $-2$, which keeps both coefficients real.

With respect to the remaining boundary conditions, the [EMRobinBC.md] object is used
in a "port" configuration at the left boundary and in an "absorbing" configuration at
the right boundary. The wavenumber along the direction of propagation is
$k_\mathrm{SPP}$, set through `coeff_real` as $-k_\mathrm{SPP}$ with
`sign = positive` for the port and $+k_\mathrm{SPP}$ with `sign = negative` for the
absorber. The incoming wave shape across the port needed by [EMRobinBC.md] will be
set to

!equation id=spp-incident
f_\mathrm{inc}(y) =
\begin{cases}
e^{-\kappa_d y} & y \geq 0, \\
e^{+\kappa_m y} & y < 0,
\end{cases}

through `profile_func_real`. The top and bottom boundaries are placed far enough
from the interface that the evanescent field has decayed to a negligible amplitude.
No condition is imposed there, so the weak form enforces a natural (homogeneous
Neumann) condition.

The first-order Robin condition is derived for plane waves and is most accurate on
spherical boundaries [!citep](jin-fem). On the planar right boundary it absorbs the
single known SPP mode, but a small residual reflection is expected.

## Analytic Solution for Comparison

!style halign=left
In order to perform a quantitative comparison, the analytical SPP field is needed
[!citep](raether1988, maier2007). The SPP wavenumber along the interface is

!equation id=spp-kspp
k_\mathrm{SPP} = k_0 \sqrt{\frac{\varepsilon_m \, \varepsilon_d}{\varepsilon_m + \varepsilon_d}}

and the field is

!equation id=spp-solution
E_x(x, y) \propto e^{i k_\mathrm{SPP} x} \, e^{-\kappa_j |y|}, \qquad \kappa_j = \sqrt{k_\mathrm{SPP}^2 - \varepsilon_j k_0^2}

where

- $j = m$ for $y < 0$ and $j = d$ for $y > 0$,
- $\kappa_j$ is the evanescent decay constant in each medium, and
- $\delta_j = 1/\kappa_j$ is the corresponding penetration depth.

Equation [spp-flux] is satisfied by [spp-solution] as a consequence of the dispersion
relation [spp-kspp]. For the parameters in [spp-benchmark-parameters],
$k_\mathrm{SPP} = 0.6283\,\ell^{-1}$ ($\lambda_\mathrm{SPP} = 10.00\,\ell$),
$\kappa_m = 0.8886\,\ell^{-1}$ ($\delta_m = 1.125\,\ell$), and
$\kappa_d = 0.4443\,\ell^{-1}$ ($\delta_d = 2.251\,\ell$).

## Mesh

!style halign=left
The meshes used in this study are generated directly in the input file with
`GeneratedMeshGenerator`, covering $0 \le x \le 20\,\ell$ and
$-20\,\ell \le y \le 20\,\ell$ (that is, $2\lambda_\mathrm{SPP} \times 4\lambda_\mathrm{SPP}$).
`SubdomainBoundingBoxGenerator` defines the metal (block 1) and the dielectric
(block 2), and `SideSetsBetweenSubdomainsGenerator` creates the `primary_interface`
sideset used by the interface conditions. The mesh convergence study uses
$80\times160$, $160\times320$, and $320\times640$ elements, and the test input uses
the coarsest of these.

## Input File

!listing SPP_flat_interface.i

The linear system is indefinite and non-Hermitian because $k_j^2$ changes sign across
the interface, so it is solved in a single steady-state solve with a direct LU
factorization.

## Results and Discussion

!style halign=left
The numerical solution is compared with [spp-solution] in two ways: $E_\mathrm{real}$
along the lines $y = +5\,\ell$ (dielectric) and $y = -5\,\ell$ (metal), which tests
the wavelength and amplitude along the direction of propagation, and $|E_x|$ along
$y$ at $x = 5\,\ell$, which tests the evanescent decay. A comparison of
$E_\mathrm{real}$ and of the decay profile for the three meshes is shown in
[spp-benchmark-verification].

!media spp_flat_interface_verification.png
       style=width:100%;margin:auto;
       id=spp-benchmark-verification
       caption=Mesh convergence of $E_\mathrm{real}$ at $y = \pm 5\,\ell$ against the analytical solution, and the evanescent decay of $\lvert E_x \rvert$ at $x = 5\,\ell$ with exponential fits.

The SPP wavelength is extracted from successive zero crossings of $E_\mathrm{real}$,
using linear interpolation between mesh points. This gives
$\lambda_\mathrm{SPP} = 9.88\,\ell$ at $y = +5\,\ell$ and $9.91\,\ell$ at
$y = -5\,\ell$, compared with the analytical $10.00\,\ell$, deviations of $1.2\%$ and
$0.9\%$. Exponential fits to $|E_x|$ at $x = 5\,\ell$ give
$\kappa_m = 0.892\,\ell^{-1}$ ($\delta_m = 1.12\,\ell$), within $0.4\%$ of the
analytical value, and a dielectric decay constant within approximately $2\%$ of the
analytical $\kappa_d$.

The root-mean-square (RMS) error with respect to the analytical solution for each
mesh is listed in [spp-benchmark-rms].

!table id=spp-benchmark-rms caption=RMS error of the numerical solution relative to the analytical solution.
| Mesh | $E_\mathrm{real}$, $y = +5\,\ell$ | $E_\mathrm{real}$, $y = -5\,\ell$ | $\lvert E_x \rvert$ at $x = 5\,\ell$ |
| - | - | - | - |
| $80\times160$ | $2.489\times10^{-2}$ | $2.313\times10^{-3}$ | $1.070\times10^{-2}$ |
| $160\times320$ | $1.222\times10^{-2}$ | $1.127\times10^{-3}$ | $5.340\times10^{-3}$ |
| $320\times640$ | $6.077\times10^{-3}$ | $5.588\times10^{-4}$ | $2.622\times10^{-3}$ |

The RMS error decreases by a factor of approximately 2 with each mesh refinement, an
observed convergence rate of about one in the element size. This is lower than the
nominal second-order $L^2$ rate of first-order Lagrange elements for smooth
solutions, and the cause of the reduced rate has not been isolated. The first-order
Robin boundary treatment is one candidate.

The scope of this benchmark is limited. The metal is lossless ($\varepsilon_m$ is
real), the interface is flat, and the $E_x$ formulation with the interface condition
[spp-flux] is verified here only for a flat interface. We extend this scope to curved interfaces in (#34243) and further to lossy metal in subsequent work.
