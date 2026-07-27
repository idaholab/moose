# ComputeFrictionalForceLMMechanicalContact

!syntax description /Constraints/ComputeFrictionalForceLMMechanicalContact


This class represents a preliminary implementation of frictional mortar contact constraints intended to be used with Lagrange's multiplier interpolation with dual bases. The nonlinear complementarity constraints employed here are based on a primal-dual active set strategy (PDASS), see [!citep](gitterle2010finite). These constraints capture nodes in sticking and slipping states on different solution branches, and can be written as:

\begin{equation}
C_{tj}(\lambda_{j},\boldsymbol{u}, \boldsymbol{\dot{u}}) = \max({\mu({p} + c_{n}\tilde{g}_{nj}),  \mathrm{abs}({\lambda_{j} + c_t \tilde{u}_{tj}}))  \lambda_{j} - \mu \max({0,({p} + c_{n}\tilde{g}_{nj})}) (\lambda_{j} + c_t \tilde{u}_{tj})
}
\end{equation}

$p$ is the normal contact pressure, $\lambda_{j}$ is a Lagrange's multiplier that refers to the tangential contact pressure at node $j$, $\tilde{u}_{tj}$ is the weighted tangential velocity integrated forward in time, $\tilde{g}_n)_j$ is the weighted normal gap, $c_{n}$ is a numerical parameter ($c$ in [ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md)) and $c_{t}$ is a numerical parameter that can determine convergence properties but has no effect on the results.

The nodal, weighted tangential velocity is computed as
\begin{equation}
\tilde{v}_{tj} = \int_{\gamma_c^{(1)}} \Phi_j v_{t,h} dA
\end{equation}

where $\gamma_c^{(1)}$ denotes the secondary contact interface, $\Phi_j$ is the
j'th lagrange multiplier test function, and $v_{t,h}$ is the discretized version
of the tangential velocity function.

This object automatically enforces normal contact constraints by making calls to its parent class `ComputeWeightedGapLMMechanicalContact`, see [ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md) for input parameters and details.

The preliminary recommendation is to select  `c` to be on the order of the moduli of elasticity of the bodies into contact, and `c_t` to be a few orders of magnitude less than `c`. This selection of these purely numerical parameters can represent an initial difficulty when running *new* models, but they can be held constant once good convergence behavior has been attained.

## Coulomb-friction regularization

The mutually exclusive parameters
[!param](/Constraints/ComputeFrictionalForceLMMechanicalContact/friction_elastic_slip) and
[!param](/Constraints/ComputeFrictionalForceLMMechanicalContact/friction_coefficient_regularization)
require matching first-order nodal normal and tangential multiplier spaces. Both use the normalized,
weakly integrated current-minus-old relative displacement at corresponding mortar points:

\begin{equation}
\Delta \boldsymbol{g}_t=\boldsymbol{P}_{t,n+1}
\left[(\boldsymbol{u}_{s,n+1}-\boldsymbol{u}_{s,n})
-(\boldsymbol{u}_{p,n+1}-\boldsymbol{u}_{p,n})\right].
\end{equation}

Here $s$ and $p$ denote the secondary and primary sides. Disabling both options preserves the
existing LM-PDASS and `function_friction` paths.

### Slip-increment coefficient regularization

`friction_coefficient_regularization = ARCTAN_SLIP` replaces the supplied coefficient $\mu$ in the
existing LM-PDASS Coulomb bound with

\begin{equation}
\mu_{\mathrm{eff}}=\mu\frac{2}{\pi}
\tan^{-1}\left(\frac{\|\Delta\boldsymbol{g}_t\|}{s_{\mathrm{ref}}}\right).
\end{equation}

The scale $s_{\mathrm{ref}}$ is
[!param](/Constraints/ComputeFrictionalForceLMMechanicalContact/friction_reference_slip).
This is increment dependent rather than a rate-independent static-friction law:
$\mu_{\mathrm{eff}}(0)=0$, so exact static sticking is relaxed; load-step partitioning can change
the result, and
$s_{\mathrm{ref}}\rightarrow0$ recovers $\mu$ for fixed nonzero slip. Its normalized residual is

\begin{equation}
\boldsymbol{R}_t=\boldsymbol{\lambda}_t-
\frac{B}{\max(B,\|\boldsymbol{T}\|)}\boldsymbol{T},\qquad
\boldsymbol{T}=\boldsymbol{\lambda}_t+c_t\Delta\boldsymbol{g}_t,\qquad
B=\mu_{\mathrm{eff}}\max(0,p+c_n\widetilde{g}_n).
\end{equation}

At $B=0$, $\boldsymbol{R}_t=\boldsymbol{\lambda}_t$ retains a unit tangential-LM diagonal. Otherwise
this residual has the original PDASS roots, although its changed scale can require retuning
[!param](/Contact/ContactAction/tangential_lm_scaling).

### Stateful elastic slip

For `friction_elastic_slip = s_e > 0`, MOOSE stores accepted recoverable tangential-gap components
in a material frame $\boldsymbol{Q}=[\boldsymbol{a}_1,\boldsymbol{a}_2,\boldsymbol{n}]$ attached to
the displaced secondary surface. The next trial state is

\begin{equation}
\boldsymbol{g}_{t}^{e,\mathrm{tr}}=
\boldsymbol{Q}_{n+1}\boldsymbol{Q}_{n}^{T}\boldsymbol{g}_{t,n}^{e}+\Delta\boldsymbol{g}_t.
\end{equation}

\begin{equation}
\tau_c=\mu\max(0,\lambda_n),\qquad K_t=\frac{\tau_c}{s_e},\qquad
\boldsymbol{\lambda}_{t}^{\mathrm{tr}}=K_t\boldsymbol{g}_{t}^{e,\mathrm{tr}}.
\end{equation}

The trial multiplier is retained when $\|\boldsymbol{g}_{t}^{e,\mathrm{tr}}\|\le s_e$; otherwise
radial return gives

\begin{equation}
\boldsymbol{\lambda}_t=\tau_c
\frac{\boldsymbol{\lambda}_{t}^{\mathrm{tr}}}{\|\boldsymbol{\lambda}_{t}^{\mathrm{tr}}\|},
\qquad \boldsymbol{g}_{t}^{e}=\frac{\boldsymbol{\lambda}_t}{K_t},\qquad
\|\boldsymbol{g}_{t}^{e}\|=s_e.
\end{equation}

This hybrid normal-LM/tangential-compliance state is reversible in stick, cleared on opening, and
committed only after an accepted timestep. The Jacobian freezes the frame and mortar normalization,
giving a quasi-Newton geometric linearization. History is keyed by secondary mortar node ID, so the
mortar mesh must remain fixed and recovery requires unchanged node numbering and MPI partition.
The allowable reversible slip is analogous to the
[Abaqus elastic-slip control](https://docs.software.vt.edu/abaqusv2024/English/SIMACAEITNRefMap/simaitn-c-friction.htm#simaitn-c-friction-slip__simaitn-c-aelasticslip),
but the equations above define the MOOSE model. Choose $s_e$ relative to local contact-segment size
and reduce it to check sensitivity; smaller values approach exact stick while increasing $K_t$.

!syntax parameters /Constraints/ComputeFrictionalForceLMMechanicalContact

!syntax inputs /Constraints/ComputeFrictionalForceLMMechanicalContact

!syntax children /Constraints/ComputeFrictionalForceLMMechanicalContact
