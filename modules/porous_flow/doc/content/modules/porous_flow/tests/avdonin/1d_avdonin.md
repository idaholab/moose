# 1D heat and mass transport

## Description

An analytical solution to the problem of 1D coupled heat and mass transport was initially
developed by [!citet](avdonin1964), and later by [!citet](ross1982).

The problem consists of a 1D model where cold water is injected into a warm semi-infinite reservoir
at a constant rate. The top and bottom surfaces of the reservoir are bounded by caprock which is
neglected in the modelling to simplify the problem. Instead, these boundaries are treated as no-flow
and adiabatic boundary conditions.

For the simple case of a 1D Cartesian model bounded on the upper and lower surfaces by no-flow
and adiabatic boundaries, a simplified solution for the temperature profile $T(x, t)$ can be obtained
[!citep](updegraff1989)

\begin{equation}
\frac{T(x, t) - T(x, 0)}{T(0, t) - T(x, 0)} = \frac{1}{2} \left[\mathrm{erfc}\left(\frac{x - vt}
  {\sqrt{4 D t}}\right) + \exp\left(\frac{vx}{D}\right) \mathrm{erfc}\left(\frac{x+vt}
  {\sqrt{4 D t}}\right)\right],
\label{eq:avdonin}
\end{equation}

where $T(x, 0)$ is the initial temperature in the reservoir, $T(0, t)$ is the temperature of the injected
water,

\begin{equation}
v = \frac{u_w \rho_w cp_w}{\rho_m cp_m},
\end{equation}

and

\begin{equation}
D = \frac{\lambda_m}{\rho_m cp_m},
\end{equation}

where $u_w$ is the Darcy velocity of the water, $\rho_w$ is the density of water, $cp_w$ is the specific
heat capacity of water, $\rho_m$ is the density of the fully saturated medium ($\rho_m = \phi \rho_w + (1 - \phi \rho_r)$ where $\phi$ is porosity and $\rho_r$ is the density of the dry rock), $cp_m$ is the specific
heat capacity of the fully saturated porous medium, and $\lambda_m$ is the thermal conductivity of the fully
saturated reservoir.

## Model

This problem was considered in a code comparison by [!citet](updegraff1989), so we use identical parameters
in this verification problem, see [tab:res].

!table id=tab:res caption=Model properties
| Property |  Value |
| - | - |
| Length | 50 m |
| Pressure | 5 MPa |
| Temperature | 170 $^{\circ}$C |
| Permeability | $1.8 \times 10^{-11}$ m$^2$ |
| Porosity | 0.2 |
| Saturate density | 2,500 kg m$^{-3}$ |
| Saturated thermal conductivity | 25 W m$^{-1}$ K |
| Saturated specific heat capacity | 1,000 J kg$^{-1}$ K |

Following [!citet](updegraff1989), a constant fluid flow through the left boundary is obtained by
applying a constant pressure gradient over the model by fixing porepressure at the boundaries. The
temperature of the water entering the model is fixed at 160 $^{\circ}$C by fixing enthalpy at the
left boundary.

## Input file

The input file used to run this problem is

!listing modules/porous_flow/test/tests/fluidstate/coldwater_injection.i

Note that the test file is a reduced version of this problem. To recreate these results, follow the
instructions at the top of the input file.

## Results

The results for the temperature profile after 13,000 seconds are shown in [fig:avdonin]. Good agreement is
shown, however some numerical diffusion is obvserved. Similar results are obtained using TOUGH2 for upstream
weighting, see [!citet](moridis1992).

!media media/porous_flow/1d_avdonin.png
       id=fig:avdonin
       style=width:60%;margin-left:10px;
       caption=Comparison between [!citet](avdonin1964) result and MOOSE at t = 13,000 s.

!bibtex bibliography
