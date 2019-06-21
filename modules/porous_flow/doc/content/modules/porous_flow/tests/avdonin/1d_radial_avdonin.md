# 1D radial heat and mass transport

## Description

An analytical solution to the problem of 1D radial coupled heat and mass transport was initially
developed by [!citet](avdonin1964), and later by [!citet](ross1982) (in a similar fashion as for the
1D Cartesian [model](1d_radial_avdonin.md)).

The problem consists of a 1D radial model where cold water is injected into a warm semi-infinite reservoir
at a constant rate. The top and bottom surfaces of the reservoir are bounded by caprock which is
neglected in the modelling to simplify the problem. Instead, these boundaries are treated as no-flow
and adiabatic boundary conditions.

For the simple case of a 1D radial model bounded on the upper and lower surfaces by no-flow
and adiabatic boundaries, a simplified solution for the temperature profile $T(r, t)$ can be obtained
[!citep](updegraff1989)

\begin{equation}
\frac{T(r, t) - T(r, 0)}{T(0, t) - T(r, 0)} = \frac{\Gamma(\nu, \omega^2/(4 \tau))}{\Gamma(\nu)},
\label{eq:avdonin}
\end{equation}

where $T(r, 0)$ is the initial temperature in the reservoir, $T(0, t)$ is the temperature of the injected
water, $\Gamma(x)$ is the gamma function, $\Gamma(x, a)$ is the lower incomplete gamma function,

\begin{equation}
\nu = \frac{Q\rho_w cp_w}{4 \pi h \lambda_m},
\end{equation}

\begin{equation}
\omega = \frac{2 r}{h},
\end{equation}

and

\begin{equation}
\tau = \frac{4 \lambda_m t}{\rho_m cp_m h^2},
\end{equation}

where $\rho_w$ is the density of water, $cp_w$ is the specific heat capacity of water, $\rho_m$ is the density
of the fully saturated medium ($\rho_m = \phi \rho_w + (1 - \phi \rho_r)$ where $\phi$ is porosity and
$\rho_r$ is the density of the dry rock), $cp_m$ is the specific heat capacity of the fully saturated porous
medium, $\lambda_m$ is the thermal conductivity of the fully saturated reservoir, $Q$ is the volumetric flow
rate, and $h$ is the height of the reservoir.

## Model

This problem was considered in a code comparison by [!citet](updegraff1989), so we use identical parameters
in this verification problem, see [tab:res].

!table id=tab:res caption=Model properties
| Property |  Value |
| - | - |
| Length | 1,000 m |
| Pressure | 5 MPa |
| Temperature | 170 $^{\circ}$C |
| Permeability | $1.8 \times 10^{-11}$ m$^2$ |
| Porosity | 0.2 |
| Saturate density | 2,500 kg m$^{-3}$ |
| Saturated thermal conductivity | 25 W m$^{-1}$ K |
| Saturated specific heat capacity | 1,000 J kg$^{-1}$ K |
| Mass flux rate | 0.1 kg s${-1}$ |

## Input file

The input file used to run this problem is

!listing modules/porous_flow/test/tests/fluidstate/coldwater_injection_radial.i

Note that the test file is a reduced version of this problem. To recreate these results, follow the
instructions at the top of the input file.

## Results

The results for the temperature profile after 13,000 seconds are shown in [fig:avdonin]. Excellent agreement
between the analytical solution and the MOOSE results are observed.

!media media/porous_flow/1d_radial_avdonin.png
       id=fig:avdonin
       style=width:80%;margin-left:10px;
       caption=Comparison between [!citet](avdonin1964) result and MOOSE at $t = 10^6$ s (left); and
       $t = 10^9$ s (right).

This model also admits a similarity solution $\zeta = r^2/t$ [!citep](moridis1992). Again, excellent
agreement between the analytical solution and the MOOSE results are observed, see [fig:avdonin_sim]

!media media/porous_flow/1d_radial_avdonin_similarity.png
       id=fig:avdonin_sim
       style=width:60%;margin-left:10px;
       caption=Similarity solution for 1D radial problem.

!bibtex bibliography
