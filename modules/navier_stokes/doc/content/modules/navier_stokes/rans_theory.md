# Turbulence modeling

## Introduction

The MOOSE Navier-Stokes module includes experimental turbulence modeling
capabilities. Presently, the models are insufficient for stand-alone predictive
simulation, and we recommend that the user tunes the model parameters for their
problem of interest using experimental data or a higher-fidelity code for
reference solutions.

## Reynolds averaging

One common approach in turbulent flow modeling is to use the Reynolds averaging
procedure. In this procedure, all of the relevant variables are decomposed into
an average value and a fluctuating value. For example, velocity will be
decomposed as $\bm{v} = \overline{\bm{v}} + \bm{v}'$ where $\overline{\bm{v}}$
is the average value and $\bm{v}'$ is the fluctuating component. It is difficult
to rigorously define what is meant by "average" and what is meant by
"fluctuating", but these concepts can be roughly defined as: the average
component is the component that we intend to explicitly resolve with the
numerical solver and the fluctuating component is the remainder which varies on
a scale that is smaller than the solver mesh size and/or faster than the solver
time step. For a more detailed discussion, refer to the books by Bailly and
Comte-Bellot [!citep](bailly2015_ch2) or Moukalled et al.
[!citep](moukalled2016_ch17).

The averaging procedure can also be understood by the relations that it obeys
which include,
\begin{equation}
  \overline{a + b} = \overline{a} + \overline{b}
\end{equation}
\begin{equation}
  \overline{\overline{a} b} = \overline{a}\,\overline{b}
\end{equation}
\begin{equation}
  \overline{\frac{\partial a}{\partial t}}
  = \frac{\partial \overline{a}}{\partial t}
\end{equation}
\begin{equation}
  \overline{a'} = 0
\end{equation}
\begin{equation}
  \overline{\overline{a} b'} = 0
\end{equation}

Equations for turbulent flow can be derived by applying the Reynolds averaging
procedure to the relevant equations. As an example, consider the conservation of
linear momentum in the $x$-direction which can be written as,
\begin{equation}
  \frac{\partial \rho v_x}{\partial t} + \nabla \cdot \rho v_x \bm{v} = f_x
\end{equation}
where $\rho$ is the fluid density, $v$ is the velocity, and $f_x$ is the
$x$-component of the force felt by the fluid.

The first step in the Reynolds averaging process is to apply the decomposition.
Here the analysis will be restricted to incompressible flows which means
$\rho' = 0$ so $\overline{\rho} = \rho$. Applying the decomposition to
the other variables gives,
\begin{equation}
  \frac{\partial \rho \left( \overline{v_x} + v_x' \right)}{\partial t}
  + \nabla \cdot \rho \left( \overline{v_x} + v_x' \right)
  \left( \overline{\bm{v}} + \bm{v}' \right) = \overline{f_x} + f_x'
\end{equation}

Because the solver will only resolve the average behavior, the next step is to
apply the averaging operator to each term,
\begin{equation}
  \frac{\partial \rho \overline{v_x}}{\partial t}
  + \nabla \cdot \rho \overline{v_x}\,\overline{\bm{v}}
  + \nabla \cdot \rho \overline{ v_x' \bm{v}' }
  = \overline{f_x}
\end{equation}
\begin{equation}
  \frac{\partial \rho \overline{v_x}}{\partial t}
  + \nabla \cdot \rho \overline{v_x}\,\overline{\bm{v}}
  = \overline{f_x}
  - \nabla \cdot \rho \overline{ v_x' \bm{v}' }
  \label{eq:rans_mom_x}
\end{equation}

The quantity, $-\rho \overline{v_x' \bm{v}'}$, seen in [eq:rans_mom_x]
is known as the Reynolds stress. It models a change in
the resolved momentum due to a covariance in the unresolved velocity field.

A model is needed to relate the Reynolds stress to known quantities in the
system (like the average velocity). One model frequently used in engineering
analysis is the Boussinesq hypothesis which treats the Reynolds stress with a
functional form similar to the one used for viscous stress.

## Boussinesq hypothesis and turbulent momentum transfer

Forces due to viscous friction can be analyzed in terms of a viscous stress
tensor, $\bm{\tau}$,
\begin{equation}
  \bm{f}_{\text{visc}} = \nabla \cdot \bm{\tau}
\end{equation}

For a Newtonian fluid, the stress tensor is given by,
\begin{equation}
  \bm{\tau} = \mu \left[ \nabla \bm{v} + \left( \nabla v \right)^T
  - \frac{2}{3} \left( \nabla \cdot \bm{v} \right) \bm{I} \right]
  + \mu_b \left( \nabla \cdot \bm{v} \right) \bm{I}
\end{equation}
where $\mu$ is the dynamic viscosity, $\mu_b$ is the bulk viscosity, and $I$ is
the identity matrix.

The Boussinesq hypothesis models the Reynolds stress in the analogous form,
[!citep](bailly2015_ch2, moukalled2016_ch17)
\begin{equation}
  -\rho \overline{\bm{v}' \bm{v}'}
  = \mu_t \left[ \nabla \overline{\bm{v}}
  + \left( \nabla \overline{\bm{v}} \right)^T \right]
  - \frac{2}{3} \rho k_t \bm{I}
\end{equation}
where $\mu_t$ is the eddy viscosity and $k_t$ is the turbulent kinetic energy
which is itself defined by,
\begin{equation}
  k_t = \frac{1}{2} \left( \overline{\bm{v}' \cdot \bm{v}'} \right)
\end{equation}

The Boussinesq hypothesis can also be stated in terms of $\epsilon_m$, the eddy
diffusivity for momentum as,
\begin{equation}
  -\rho \overline{\bm{v}' \bm{v}'}
  = \rho \epsilon_m \left[ \nabla \overline{\bm{v}} +
  \left( \nabla \overline{\bm{v}} \right)^T \right]
  - \frac{2}{3} \rho k_t \bm{I}
  \label{eq:boussinesq}
\end{equation}

Note that $\epsilon_m$ is analogous to the kinematic viscosity,
$\nu = \mu / \rho$, so $\epsilon_m$ is also referred to as the eddy viscosity in
some sources.

As a practical matter, the second term on the right-hand-side of [eq:boussinesq]
is frequently neglected and implicitly subsumed into the pressure term of the
momentum equation. The new pressure variable can be referred to as the
turbulent pressure instead of the thermodynamic pressure
[!citep](moukalled2016_ch17). Neglecting this term results in the model,
\begin{equation}
  -\rho \overline{\bm{v}' \bm{v}'}
  = \rho \epsilon_m \left[ \nabla \overline{\bm{v}}
  + \left( \nabla \overline{\bm{v}} \right)^T \right]
  \label{eq:boussinesq2}
\end{equation}

This stress model is implemented in the [INSFVMixingLengthReynoldsStress](source/fvkernels/INSFVMixingLengthReynoldsStress.md) kernel.

## Turbulent energy transfer

A model for the effects of turbulence on the conservation of energy can be
derived by following the same process that was used for the conservation of
momentum. First, the Reynolds-averaging procedure is applied to the conservation
equation. Then, the flux due to the fluctuating covariance term is replaced with
a model analogous to the one used for molecular processes.

The conservation of energy can be expressed as
\begin{equation}
  \frac{\partial \rho c_p T}{\partial t} + \nabla \cdot \rho c_p T \bm{v}
  + \nabla \cdot q''_\text{cond} = Q_T
\end{equation}
where $c_p$ is the specific heat capacity, $q''_\text{cond}$ is the
heat flux due to molecular conduction, and $Q_T$ captures any sources and losses
of heat. Note that this equation only conserves energy for the case of constant
$\rho$ and $c_p$.

The conduction term can be modeled with Fourier's law as
\begin{equation}
  q''_\text{cond} = -\lambda \nabla T
\end{equation}
where $\lambda$ is the thermal conductivity.

Substituting in Fourier's law gives the form,
\begin{equation}
  \frac{\partial \rho c_p T}{\partial t} + \nabla \cdot \rho c_p T \bm{v}
  - \nabla \cdot \lambda \nabla T = Q_T
\end{equation}

The Reynolds averaging procedure can now be applied to find,
\begin{equation}
  \frac{\partial \rho c_p \overline{T}}{\partial t}
  + \nabla \cdot \rho c_p \overline{T \bm{v}}
  - \nabla \cdot \lambda \nabla \overline{T} = \overline{Q_T}
\end{equation}
\begin{equation}
  \frac{\partial \rho c_p \overline{T}}{\partial t}
  + \nabla \cdot \rho c_p \overline{T}\,\overline{\bm{v}}
  + \nabla \cdot \rho c_p \overline{T' \bm{v}'}
  - \nabla \cdot \lambda \nabla \overline{T} = \overline{Q_T}
\end{equation}

As before with the Boussinesq hypothesis for turbulent momentum transfer, we can
model turbulent heat transfer with an analogy to the molecular mechanism,
\begin{equation}
  \rho c_p \overline{T' \bm{v}'} = -\rho c_p \epsilon_q \nabla \overline{T}
\end{equation}
where $\epsilon_q$ is the eddy diffusivity for heat [!citep](lienhard2020_ch6).

The diffusivity for heat is related to the diffusivity for momentum by,
\begin{equation}
  \text{Pr}_t = \frac{\epsilon_m}{\epsilon_q}
\end{equation}
where $\text{Pr}_t$ is the turbulent Prandtl number [!citep](lienhard2020_ch6).

## Turbulent transfer of trace species

Some problems also require modeling the transport of trace chemicals (e.g. a
dye) in the fluid. Here, the analysis is limited to very dilute substances that
have no significant impact on bulk fluid properties like $\rho$ and $\mu$.

The conservation of these passive species can be expressed as
\begin{equation}
  \frac{\partial c}{\partial t} + \nabla \cdot c \bm{v}
  - \nabla \cdot \Gamma \nabla c = Q_c
\end{equation}
where $c$ is the concentration of the species in number of molecules (or
moles) per unit volume, $\Gamma$ is the molecular diffusion coefficient
(Fick's law has already been applied in this equation), and $Q_c$ is an
unspecified source and/or loss term.

Applying the Reynolds averaging procedure leads to the equation,
\begin{equation}
  \frac{\partial \overline{c}}{\partial t}
  + \nabla \cdot \overline{c}\,\overline{\bm{v}}
  + \nabla \cdot \overline{c' \bm{v}'}
  - \nabla \cdot \Gamma \nabla \overline{c} = \overline{Q_c}
\end{equation}

The molecular analogy is then,
\begin{equation}
  \overline{c' \bm{v}'} = -\epsilon_c \nabla \overline{c}
\end{equation}
where $\epsilon_c$ is the turbulent diffusivity for the passive species
which is in turn modeled with the relationship,
\begin{equation}
  \text{Sc}_t = \frac{\epsilon_m}{\epsilon_c}
\end{equation}
where $\text{Sc}_t$ is the turbulent Schmidt number [!citep](tominaga2007).

## Mixing-length models

There are many models used in CFD codes to compute the eddy viscosity, $\mu_t$.
Some of the most popular are the isotropic 2-equation models, $k$-$\epsilon$ and
$k$-$\omega$. However, the MOOSE Navier-Stokes module only supports simple
algebraic mixing-length models at present.

Derivations of mixing length models generally consider 2D geometry of the
near-wall region. Let $x$ be a direction tangential to the wall and $y$ be
the direction perpendicular to the wall. With this notation, the eddy
diffusivity can be modeled as, [!citep](bailly2015_ch3, todreas2011_ch10)
\begin{equation}
  \epsilon_m = -l_m^2 \bigg| \frac{\partial v_x}{\partial y} \bigg|
\end{equation}
where $l_m$ is the mixing length, a characteristic length for turbulent eddies
in the fluid.  The mixing length will be discussed further below.

This model is useful for analysis of simple geometries, but it is inconvenient
to apply for general meshes where the walls may take an arbitrary shape.  This
is because there is no clear way to define directions parallel and perpendicular
to the wall in the general case (e.g. consider a mesh cell with walls on two
sides).  Consequently, we use Smagorinsky's velocity scale:
\begin{equation}
  \epsilon_m = -l_m^2 |2 S_{ij}:S_{ij}|
\end{equation}

where $S_{ij} = 0.5 \cdot \left( \frac{\partial u_i}{\partial x_j} +
  \frac{\partial u_j}{\partial x_i} \right)$

This momentum diffusivity model is implemented in the [INSFVMixingLengthReynoldsStress](source/fvkernels/INSFVMixingLengthReynoldsStress.md)
kernel. The corresponding model for diffusivity of passive scalars (like energy)
is implemented in the [INSFVMixingLengthScalarDiffusion](source/fvkernels/INSFVMixingLengthScalarDiffusion.md) kernel.

A model is then needed for the mixing length itself. One popular choice is to
assume the mixing length is proportional to the distance from the nearest wall,
[!citep](bailly2015_ch3, todreas2011_ch10)
\begin{equation}
  l_m = k d
\end{equation}
where $d$ is the wall-distance and $k$ is known as the von Kármán constant.  A
von Kármán of $k = 0.41$ is often used for the near-wall region
[!citep](todreas2011_ch10).

A modification to this model was done by [!citep](escudier1966), who claims
that the mixing length grows linearly in the boundary layer region and then it
takes a constant value. This prevents an excessive growth of Prandtl's original
mixing length model. The equations for the mixing length are then

\begin{equation}
  l_m = \kappa y_d \quad if \: \kappa y_d < \kappa_0 \delta \\
  l_m = \kappa_0 \delta \quad if \: \kappa y_d \geq \kappa_0 \delta
\end{equation}

where $\kappa = 0.41$ is the Von Karman constant, $\kappa_0 = 0.09$ as in
Escudier's model and $\delta$ has length units and represents the thickness of
the velocity boundary layer. Note that for large values of $\delta$, the mixing
length in Prandtl's original model is obtained.

This mixing length is implemented in the [WallDistanceMixingLengthAux](source/auxkernels/WallDistanceMixingLengthAux.md)
auxiliary kernel. Note that the wall-distance calculation can be expensive for
large meshes. If the mesh is constant in time, this cost can be amortized by
setting the `execute_on` parameter to `initial` so that the wall distance is
computed only once at the beginning of the simulation.

## Tuning the mixing length

In practice, the mixing length distribution is highly problem-dependent. There
are also known deficiencies with the mixing length model itself. For example,
this model predicts no turbulent mixing where the velocity gradient is zero.

Consequently, we recommend that users compare results generated with this model
against reference solutions (either from experiment or from software with higher
fidelity models). These comparisons can also be used to tune an appropriate
mixing length model to the system of interest.

The examples directory includes a simple circular pipe problem and shows how $k$
can be tuned so that the simulated pressure drop matches a correlation. Also
note that other MOOSE auxkernels can be used to implement different mixing
length models. For example, if the near-wall region and the core region of the
pipe are assigned to different mesh blocks, then a `WallDistanceMixingLengthAux`
can be combined with a `ConstantAux` to model a mixing length that is
proportional to the wall-distance in the near-wall region and constant
elsewhere.

## Wall Function Boundary Condition for Velocity

In wall-bounded flows, the velocity gradients present in the near wall region
are quite steep. Therefore, the mesh resolution needed to capture these
gradients can become expensive. To economize computer time and storage, wall
functions have been proposed, which are equations empirically derived to satisfy
the physics in the near wall region under certain assumptions.

Experimental and dimensional analysis shows that for high $y^+$ values
($y^+>30$) the wall shear stress $\tau_w$ is related to the mean velocity
parallel to the wall through the so-called logarithmic law of the wall:
[!citep](moukalled2016_ch17) [!citep](launder1983numerical)

\begin{equation}
v^{+}_t = \frac{1}{\kappa} ln(Ey^+)
\end{equation}

where:

- $v_t ^+ = \frac{|v_t|}{v_{\tau}}$ is the dimensionless wall-tangential velocity component.
- $v_t$ is the wall-tangential velocity component at the centroid of the adjacent cell to the wall.
- $v_{\tau} = \sqrt{\frac{|\tau_w|}{\rho}}$ is the friction velocity.
- $y^+ = \frac{y_d v_{\tau}}{\nu}$ is the dimensionless wall distance
- $\kappa$ is the von Karman constant
- $E$ is the log law offset

The log law offset is set for smooth walls to a value of 9.0
[!citep](launder1983numerical).Theoretically, in the viscous sublayer for
low $y^+$ values ($y^+<5$), the functional form of the velocity is linear
 $v^{+}_t=y^+$. The log law offset is obtained by matching the viscous sublayer
 profile with the logarithmic law of the wall at $y^+=11.25$.
 [!citep](launder1983numerical). Rough walls are not currently supported.
  Nonetheless, this could be done through a modification of the log law offset.

The logarithmic law of the wall provides an implicit equation for the magnitude
of the wall shear stress. The orientation of the wall shear stress is dictated
by the orientation of the tangential velocity, which is parallel to the wall.
Its sense is opposite to the velocity's sense. Its implementation is present in
[INSFVWallFunctionBC](source/fvbcs/INSFVWallFunctionBC.md).

This shear stress is used in solving the momentum equation by invoking its
value as a flux source term. The second condition needed is the
non-penetrability of the velocity through the wall, such that the mass flux
through that face is zero [!citep](segal1993isnas).

The location of the first cell centroid must be such that $y^+>11.25$. Auxiliary
Kernel [WallFunctionYPlusAux](source/auxkernels/WallFunctionYPlusAux.md) can
be used to obtain the $y^+$ value. Another Auxiliary Kernel allows to obtain the
wall shear stress, [WallFunctionWallShearStressAux](source/auxkernels/WallFunctionWallShearStressAux.md).

This standard wall function was originally calibrated for fully developed
turbulent flows in steady state, in absence of an external force and an adverse
pressure gradient. It may not provide accurate results under separation,
recirculation or low Reynolds number flows.




!bibtex bibliography
