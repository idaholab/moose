# PCNSFVHLLC

The derivation of the porous HLLC discretization that follows is based
extensively on the material in [!citep](toro2009riemann), drawing mostly from
chapters 2, 3, and 10. Details pertinent to the MOOSE
implementation of the free-flow
HLLC discretization can be found in [CNSFVHLLCBase.md].

## Solution Properties Across the Contact Wave

HLLC restores the middle contact wave to the HLL formulation. Generalized
Riemann Invariants reveal what quantities change or are constant across the
wave. We perform the Generalized Riemann Invariants analysis on the porous Euler
equations in the following way: we convert the $\epsilon \nabla p$ term into $\nabla
\cdot\left(\bm{I}\epsilon p\right) - p \nabla \epsilon$ and ignore the latter
term when composing the flux vector $\bm{F}$ (the term is instead treated as
part of a source vector $\bm{S}$). Then we define our conserved variable set
(for one-dimension for simplicity here)

\begin{equation}
\label{eq:conserved_variable_set}
\bm{U} =
\begin{bmatrix}
\rho \epsilon\\
\rho \epsilon u\\
\rho \epsilon e_t
\end{bmatrix}
\end{equation}

and our flux vector

\begin{equation}
\bm{F} =
\begin{bmatrix}
\rho \epsilon u\\
\rho \epsilon u^2 + \epsilon p\\
\rho \epsilon u  \left(e_t + \frac{p}{\rho}\right)
\end{bmatrix}
\end{equation}

and our source vector

\begin{equation}
\bm{S} =
\begin{bmatrix}
0\\
p \nabla \epsilon\\
0
\end{bmatrix}
\end{equation}

Armed with these definitions we can write the Euler equations succintly as

\begin{equation}
\label{eq:general_euler}
\bm{U}_t + \bm{F}_x = \bm{S}.
\end{equation}

We can also write [eq:general_euler] in a quasi-linear form

\begin{equation}
\bm{U}_t + \bm{A}\bm{U}_x = \bm{S}
\end{equation}

where $\bm{A}$ is the Jacobian matrix of partial derivatives of $\bm{F}$ with
respect to $\bm{U}$. It can be shown for an ideal gas that the eigenvalues of $\bm{A}$ are

\begin{equation}
\lambda_1 = u - c\ ,\ \lambda_2 = u\ ,\ \lambda_3 = u + c
\end{equation}

where $c$ is the speed of sound in the medium. The corresponding eigenvectors
are

\begin{equation}
\label{eq:eigenvectors}
\bm{K}^{(1)} =
\begin{bmatrix}
1\\
u - c\\
h_t - uc
\end{bmatrix}
\ ,\ \bm{K}^{(2)} =
\begin{bmatrix}
1\\
u\\
\frac{1}{2}u^2
\end{bmatrix}
\ ,\ \bm{K}^{(3)} =
\begin{bmatrix}
1\\
u + c\\
h_t + uc
\end{bmatrix}
\end{equation}

where $h_t = e_t + p/\rho$ is the total specific enthalpy. The second
eigenvector $K^{(2)}$ corresponds to the middle contact wave. For a general $m
\times m$ system, with variable set:

\begin{equation}
\bm{W} = \left[w_1,w_2,\ ...\ ,w_m\right]
\end{equation}

and eigenvectors

\begin{equation}
\bm{K}^{(i)} = \left[k_1^{(i)},k_2^{(i)},\ ...\ ,k_m^{(i)}\right]
\end{equation}

the $i\text{-th}$
Generalized Riemann Invariants are given by the $\left(m - 1\right)$ ODEs:

\begin{equation}
\label{eq:gri}
\frac{dw_1}{k_1^{(i)}} = \frac{dw_2}{k_2^{(i)}} =\ ...\ = \frac{dw_m}{k_m^{(i)}}
\end{equation}

Taking our conserved variable set ([eq:conserved_variable_set]) and the contact
wave eigenvector from [eq:eigenvectors] and substituting into [eq:gri] yields
the relations

\begin{equation}
\frac{d\left(\rho\epsilon\right)}{1} = \frac{d\left(\rho\epsilon u\right)}{u} =
\frac{d\left(\rho\epsilon e_t\right)}{\frac{1}{2}u^2}
\end{equation}

These equalities can be algebraically manipulated to yield the following
relationships across the contact wave:

\begin{equation}
\label{eq:contact_relationships}
u = constant\ ,\ \epsilon p = constant
\end{equation}

We will use [eq:contact_relationships] when constructing the porous HLLC fluxes
below.

## Porous HLLC Fluxes

For discontinuous wave solutions over a wave-speed $S_i$ associated with the
$\lambda_i$ characteristic, the Rankine-Hugoniot conditions state that the flux
changes according to

\begin{equation}
\Delta \bm{F} = S_i\Delta \bm{U}
\end{equation}

We can apply the Rankine-Hugoniot conditions to help us establish a
discretization for the porous HLLC fluxes. Applying Rankine-Hugoniot conditions
over the left wave results in

\begin{equation}
\label{eq:left_wave}
\bm{F}_{*L} = \bm{F}_L + S_L\left(\bm{U}_{*L} - \bm{U}_L\right)
\end{equation}

Analogously over the center contact wave:

\begin{equation}
\label{eq:center_wave}
\bm{F}_{*R} = \bm{F}_{*L} + S_*\left(\bm{U}_{*R} - \bm{U}_{*L}\right)
\end{equation}

and the right wave

\begin{equation}
\label{eq:right_wave}
\bm{F}_R = \bm{F}_{*R} + S_R\left(\bm{U}_R - \bm{U}_{*R}\right)
\end{equation}

The star fluxes can be written $\bm{F}_{*K}$ = $\bm{F}\left(\bm{U}_{*K}\right)$,
where $K$ denots $L$ or $R$ and preliminarily we will write

\begin{equation}
\label{eq:unknown_u}
\bm{U}_{*K} =
\begin{bmatrix}
\rho_{*K}\\
\rho_{*K} u_{*K}\\
\rho_{*K} e_{t,*K}
\end{bmatrix}
\end{equation}

Our goal is to eventually express $\bm{U}_{*K}$ and consequently $\bm{F}_{*K}$ in terms of known left and right
quantities, We can leverage the information from [eq:contact_relationships] to help us
construct star region solutions

\begin{equation}
\label{eq:star_region_info}
\begin{cases}
\epsilon_L p_{*L} &= \epsilon_R p_{*R}\\
u_{*L} &= u_{*R} = u_*
\end{cases}
\end{equation}

Per [!citep](toro2009riemann) it is justifiable to select that the middle wave
speed be equal to the star region velocity

\begin{equation}
\label{eq:velocity_star_eqn}
S_* = u_*
\end{equation}

Manipulating the mass, momentum, and energy components of [eq:left_wave] and
[eq:right_wave], we can construct equations for the star region density, pressure, and
total specific energy as functions of the known left and right states and the middle wave
speed $S_*$ (where we have substituted $S_*$ anyplace we encountered $u_{*L}$ or
$u_{*R}$). The star region density relationships are given by

\begin{equation}
\label{eq:densities}
\rho_{*K} = \rho_K \frac{S_K - u_K}{S_K - S_*};
\end{equation}

the star region pressure relationships are given by

\begin{equation}
\label{eq:pressure_eqns}
p_{*K} = p_K + \rho_K\left(S_K - u_K\right)\left(S_* - u_K\right);
\end{equation}

and the total specific energy relationships are given by

\begin{equation}
\label{eq:energy_eqns}
e_{t,*K} = e_{t,K} + \left(S_* -
u_K\right)\left[S_* + \frac{p_K}{\rho_K\left(S_K - u_K\right)}\right]
\end{equation}

Substituting [eq:densities], [eq:pressure_eqns], and [eq:energy_eqns] into
[eq:unknown_u] and
using $u_* = S_*$ from [eq:velocity_star_eqn], we arrive at the vector
expression for $\bm{U}_{*K}$:

\begin{equation}
\bm{U}_{*K} = \epsilon_K \rho_K \left(\frac{S_K - u_K}{S_K - S_*}\right)
\begin{bmatrix}
1\\
S_*\\
e_{t,K} + \left(S_* -
u_K\right)\left[S_* + \frac{p_K}{\rho_K\left(S_K - u_K\right)}\right]
\end{bmatrix}
\end{equation}

We must now establish an equation for $S_*$. Combining [eq:pressure_eqns] with the pressure information from
[eq:star_region_info], we arrive at:

\begin{equation}
\label{eq:star_velocity}
S_* = \frac{\epsilon_R p_R - \epsilon_L p_L + \rho_L \epsilon_L u_L \left(S_L -
u_L\right) - \rho_R \epsilon_R u_R \left(S_R - u_R\right)}{\epsilon_L \rho_L
\left(S_L - u_L\right) - \epsilon_R \rho_R \left(S_R - u_R\right)}
\end{equation}

Left and right wave speeds $S_L$ and $S_R$ are computed in the same way as for
free flow as outlined in [CNSFVHLLCBase.md]. With $S_L$ and $S_R$ the final HLLC
flux can be constructed:

\begin{equation}
\label{eq:porous_hllc}
\bm{F}_{HLLC} =
\begin{cases}
\bm{F}_L & \textrm{if } 0 \leq S_L \\
\bm{F}_{*L} & \textrm{if } S_L \leq 0 \leq S_* \\
\bm{F}_{*R} & \textrm{if } S_* \leq 0 \leq S_R \\
\bm{F}_R & \textrm{if } 0 \geq S_R
\end{cases}
\end{equation}

Note that although in the derivation above we assumed a one-dimensional setup,
the intermediate solution states $\bm{U}_{*K}$ can be generalized to
three-dimensions in a way analogously to the multidimensional free-flow
intermediate solutions expressed in [CNSFVHLLCBase.md]. Indeed, the porous
intermediate states can be simply expressed as

\begin{equation}
\bm{U}_{*K,porous} = \epsilon_K \bm{U}_{*K,free}
\end{equation}

and with  $S_*$ expressed according to [eq:star_velocity].

The mass component of [eq:porous_hllc] is implemented in [PCNSFVMassHLLC.md],
momentum in [PCNSFVMomentumHLLC.md], and fluid energy in
[PCNSFVFluidEnergyHLLC.md].

!bibtex bibliography
