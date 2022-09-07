# ComputeFrictionalForceCartesianLMMechanicalContact

!syntax description /Constraints/ComputeFrictionalForceCartesianLMMechanicalContact


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
of the tangential velocity function. Note that this object assumes that the Lagrange multipliers are defined in a global Cartesian frame.
Local variables to solve the contact problem are projected using nodal geometry, i.e. normal and tangential vectors.

This object automatically enforces normal contact constraints by making calls to its parent class.

The preliminary recommendation is to select  `c` to be on the order of the moduli of elasticity of the bodies into contact, and `c_t` to be a few orders of magnitude less than `c`. This selection of these purely numerical parameters can represent an initial difficulty when running *new* models, but they can be held constant once good convergence behavior has been attained.

The `ComputeFrictionalForceCartesianLMMechanicalContact` object computes the weighted gap and
applies the frictional contact conditions using Lagrange multipliers defined in a global Cartesian reference frame.
As a consequence, the number of contact constraints at each node will be two, in two-dimensional problems,
and three, in three-dimensional problems. The normal contact pressure is obtained by projecting the Lagrange
multiplier vector along the normal vector computed from the mortar generation objects. The result is a normal
contact constraint, which, in general, will be a function of all (two or three) Cartesian Lagrange multipliers.
The other degree(s) of freedom are constrained by
enforcing that tangential tractions follow Coulomb constraints within a semi-smooth Newton approach. Usage of
Cartesian Lagrange multipliers is recommended when condensing Lagrange multipliers via the variable condensation preconditioner
(VCP) [VariableCondensationPreconditioner](/VariableCondensationPreconditioner.md).


!syntax parameters /Constraints/ComputeFrictionalForceCartesianLMMechanicalContact

!syntax inputs /Constraints/ComputeFrictionalForceCartesianLMMechanicalContact

!syntax children /Constraints/ComputeFrictionalForceCartesianLMMechanicalContact
