# NEML2 Material Inversion Example: Chaboche-Class Viscoplasticity

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [Example 3: Transient Solve with Automatic Adjoint](material_transient.md)
- [Example 4: NEML2 Viscoplastic Parameter Inversion](invOptNEML2Viscoplastic.md)
- [debuggingHelp.md]
- [TaoGradientTester.md]

The material model is evaluated through the [NEML2 interface](syntax/NEML2/index.md), which documents the parameter and derivative transfer syntax used below.

# Example: Four-Parameter NEML2 Viscoplastic Inversion id=sec:neml2Viscoplastic

This example recovers four scalar parameters of a Chaboche-class viscoplastic NEML2 material from
synthetic displacement measurements.  The gradient is computed by a single adjoint solve using the
[TransientAndAdjoint.md] executioner, and it is exact to machine precision.

Exact, here, has a narrow boundary.  The material is stateful: the plastic strain and the backstress
carried from one step to the next depend on the parameters being inverted.  The transient adjoint
does not propagate that dependence.  The gradient is therefore exact only when the forward solve
runs a *single* time step, which is why
[!param](/Executioner/TransientAndAdjoint/num_steps) is fixed at 1 throughout.  With more than one
step the gradient is wrong and no error is raised; see [#sec:multistep].

## Problem Setup id=sec:setup

The mesh is five disconnected single-element blocks.  Block $b$, for $b = 0 \ldots 4$, occupies
$x, y \in [0, 1]$ and $z \in [3b, 3b+1]$.  Each block carries symmetry rollers on $x = 0$, $y = 0$
and $z = 3b$, and a tensile traction in $+z$ on its top face.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=Mesh
         id=mesh
         caption=Five disconnected single-element blocks.

The tractions are 40, 80, 120, 160 and 200 against a true yield stress of 100, so blocks 0 and 1
remain elastic and blocks 2 through 4 go well into the plastic regime.  The elastic blocks constrain
the Young modulus alone; the plastic blocks separate the yield stress, the hardening modulus and the
viscous parameter.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=BCs
         id=bcs
         caption=Symmetry rollers and the five traction levels.

Because the blocks are disconnected, each one is in a homogeneous uniaxial-stress state.  That is
what makes the measurements computable outside MOOSE without solving a PDE; see [#sec:truth].

## The NEML2 Material Model id=sec:model

The model is small-strain J2 viscoplasticity with linear isotropic elasticity, one
Frederick-Armstrong kinematic backstress, Perzyna viscoplastic flow, and backward Euler integration
through `ImplicitUpdate`.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/viscoplasticity.i
         id=neml2_model
         caption=The NEML2 model file.

Stress follows from the elastic strain,

\begin{equation}\label{eq:neml2_elastic}
\boldsymbol{\sigma} = \mathbb{C}(E, \nu) : \left(\boldsymbol{\varepsilon} - \boldsymbol{\varepsilon}^p\right),
\end{equation}

the yield function is built from the von Mises norm of the overstress,

\begin{equation}\label{eq:neml2_yield}
\bar{\sigma} = \sqrt{\tfrac{3}{2}} \left\lVert \mathrm{dev}(\boldsymbol{\sigma}) - \mathbf{X} \right\rVert,
\qquad
f = \sqrt{\tfrac{2}{3}} \left( \bar{\sigma} - \sigma_y \right),
\end{equation}

flow is associative and driven by a Perzyna rate with a Macaulay bracket,

\begin{equation}\label{eq:neml2_flow}
\dot{\gamma} = \left\langle \frac{f}{\eta} \right\rangle^{n},
\qquad
\dot{\boldsymbol{\varepsilon}}^p = \dot{\gamma}\, \mathbf{N},
\qquad
\mathbf{N} = \frac{\partial f}{\partial \boldsymbol{\sigma}},
\end{equation}

and the backstress evolves without static recovery,

\begin{equation}\label{eq:neml2_backstress}
\dot{\mathbf{X}} = \left( \tfrac{2}{3} C \mathbf{N} - g \mathbf{X} \right) \dot{\gamma}.
\end{equation}

Here $\mathbb{C}$ is the isotropic elasticity tensor built from the Young modulus $E$ and the
Poisson ratio $\nu$, $\boldsymbol{\varepsilon}^p$ is the plastic strain, $\mathbf{X}$ is the
backstress, $\sigma_y$ is the yield stress, $\eta$ and $n$ are the Perzyna reference stress and
exponent, and $C$ and $g$ are the backstress hardening and dynamic-recovery coefficients.

[!eqref](eq:neml2_backstress) is the no-static-recovery form of the Chaboche law, supplied by
NEML2's `FredrickArmstrongPlasticHardening`.  NEML2 spells that class name "Fredrick", without a
second "e".  `ChabochePlasticHardening` is the wrong choice for this model: at this NEML2 pin it
declares the static-recovery parameters `A` and `a` as required, and this model deliberately has no
static recovery.

### Inverted and Fixed Parameters id=sec:params

Four parameters are inverted; three are fixed and known.  Every value below is a literal line in
[neml2_model], which is the reason the model is written in NEML2's native convention rather than a
textbook one: the input file and the parameter table cannot drift apart.

| Symbol | NEML2 parameter name | True value | Status |
| :- | :- | :- | :- |
| $E$ | `elasticity_E` | 1e5 | inverted |
| $\sigma_y$ | `yield_sy` | 100 | inverted |
| $C$ | `Xrate_C` | 1.2e4 | inverted |
| $\eta$ | `flow_rate_eta` | 500 | inverted |
| $\nu$ | `elasticity_nu` | 0.3 | fixed |
| $g$ | `Xrate_g` | 20 | fixed |
| $n$ | `flow_rate_n` | 2 | fixed |

### NEML2 and Textbook Chaboche Conventions id=sec:conventions

NEML2's yield function carries a $\sqrt{2/3}$ prefactor that the textbook form does not.  Compare
[!eqref](eq:neml2_yield) with the form found in most of the plasticity literature,

\begin{equation}\label{eq:textbook_yield}
f_{\mathrm{tb}} = \bar{\sigma} - \sigma_y,
\qquad\text{so}\qquad
f = \sqrt{\tfrac{2}{3}}\, f_{\mathrm{tb}}.
\end{equation}

The factor propagates twice.  It scales the Perzyna rate directly, and it scales the flow direction
as well, because normality differentiates the same $f$.  The textbook direction
$\mathbf{N}_{\mathrm{tb}} = \partial f_{\mathrm{tb}} / \partial \boldsymbol{\sigma}$ has norm
$\sqrt{3/2}$, so NEML2's $\mathbf{N} = \sqrt{2/3}\, \mathbf{N}_{\mathrm{tb}}$ has unit norm.  The
consequence is that NEML2's $\dot{\gamma}$ is $\lVert \dot{\boldsymbol{\varepsilon}}^p \rVert$,
whereas the textbook equivalent plastic strain rate is
$\sqrt{2/3}\, \lVert \dot{\boldsymbol{\varepsilon}}^p \rVert$.  A different scalar therefore drives
the dynamic-recovery term in [!eqref](eq:neml2_backstress).

The two formulations are equivalent, but only after reparameterizing $g$ and $\eta$.

For $g$, equate the dynamic-recovery terms.  NEML2 subtracts $g \mathbf{X} \dot{\gamma}$ while the
textbook form subtracts $g_{\mathrm{tb}} \mathbf{X} \sqrt{2/3} \lVert \dot{\boldsymbol{\varepsilon}}^p \rVert$.
Since $\dot{\gamma} = \lVert \dot{\boldsymbol{\varepsilon}}^p \rVert$, the strain-rate factor
cancels and the exponent $n$ never enters.

For $\eta$, require the same $\dot{\boldsymbol{\varepsilon}}^p$ at a given stress state.  Writing
both flow rules in terms of the textbook direction,

\begin{equation}\label{eq:eta_derivation}
\left\langle \frac{\sqrt{2/3}\, f_{\mathrm{tb}}}{\eta} \right\rangle^{n} \sqrt{\tfrac{2}{3}}\, \mathbf{N}_{\mathrm{tb}}
= \left\langle \frac{f_{\mathrm{tb}}}{\eta_{\mathrm{tb}}} \right\rangle^{n} \mathbf{N}_{\mathrm{tb}}
\qquad\Longrightarrow\qquad
\frac{\eta^{n}}{\eta_{\mathrm{tb}}^{n}} = \left(\tfrac{2}{3}\right)^{\frac{n+1}{2}},
\end{equation}

where the $(2/3)^{1/2}$ from the flow direction and the $(2/3)^{n/2}$ from the rate combine into the
exponent $(n+1)/2$.  Taking the $n$-th root gives the two mappings,

\begin{equation}\label{eq:convention_map}
g_{\mathrm{tb}} = \frac{g}{\sqrt{2/3}},
\qquad
\eta_{\mathrm{tb}} = \frac{\eta}{\left(2/3\right)^{\frac{n+1}{2n}}},
\end{equation}

which for $n = 2$ reduces to $\eta_{\mathrm{tb}} = \eta / (2/3)^{3/4}$.  $E$, $\nu$, $\sigma_y$, $C$
and $n$ are unaffected.

The asymmetry between the two mappings is itself a trap: $g$ converts by a fixed factor, but
$\eta$'s conversion depends on $n$.  A factor calibrated once at one exponent and reused at another
is wrong.

!alert warning title=Parameters taken from a Chaboche paper need conversion
Dropping published $g$ and $\eta$ values straight into a NEML2 model gives silently wrong
plasticity.  Apply [!eqref](eq:convention_map) first, using the $n$ of the model being built rather
than a factor carried over from another one.  The failure hides where a casual check does not look:
with the wrong convention, the elastic blocks in this example agree exactly and only the plastic
blocks are off, by 9 to 12 percent.

This example uses NEML2's native convention throughout.

## Driving Four Parameters Through One Adjoint Solve id=sec:wiring

Every stage of the chain is already list-valued, so inverting $N$ parameters instead of one requires
no new C++.  $N$ [!param](/NEML2/parameter_derivatives) pairs auto-instantiate $N$ derivative
material properties, which feed $N$ [AdjointStrainSymmetricStressGradInnerProduct.md] instances,
whose reporter groups concatenate into the vector TAO sees.

The `[NEML2]` block names the model, the parameters to drive, and the derivatives to retrieve.
Pairs in [!param](/NEML2/derivatives) and [!param](/NEML2/parameter_derivatives) are delimited by
semicolons.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=NEML2
         id=neml2_block
         caption=Four parameter-derivative pairs from one NEML2 model.

Each pair produces a material property named `d<output>/d<parameter>`, so the four pairs in
[neml2_block] declare

- `dneml2_stress/delasticity_E`
- `dneml2_stress/dyield_sy`
- `dneml2_stress/dXrate_C`
- `dneml2_stress/dflow_rate_eta`

### Normalized Parameters id=sec:normalization

Two parametrizations are in play, and every number on this page belongs to one of them.  TAO steps a
dimensionless vector $\hat{\mathbf{p}}$, while NEML2 receives physical values.  A fixed scale relates
them,

\begin{equation}\label{eq:normalization}
p_k = p_k^0\, \hat{p}_k,
\end{equation}

where $p_k^0$ is the starting guess for parameter $k$.  Every $\hat{p}_k$ therefore starts at 1.

The reason is conditioning.  In physical units the gradient components span a factor of about 3900,
because the parameters themselves span $10^2$ to $10^5$; an optimizer stepping a vector whose
components differ that much has no single step size that suits all four.  In normalized units the
same four components span a factor of about 4.  Nothing about the physics, the model or the
evaluation point changes; only the variable TAO steps does.

The scale lives in the `Functions` block, one [ParsedOptimizationFunction.md] per parameter carrying
its own $p_k^0$ as a literal factor: `8.0e4 * E`, `85 * sigma_y`, `8.0e3 * C` and `400 * eta` in this
case.  See [functions].

Scaling by the starting guess rather than by round numbers near the truth is deliberate.  Scale
factors chosen near the answer would put the answer in the input file, and a reader would be right to
ask why they happen to be the values being recovered.  Scaling by the guess leaks nothing about the
truth, which matters for an example whose point is that the truth is computed independently
(see [#sec:truth]).

The wiring needs no new C++, because the scale is already differentiated.
[ParsedOptimizationFunction.md] is an `OptimizationFunction`, so it supplies a `parameterGradient`,
and the inner-product helper behind [AdjointStrainSymmetricStressGradInnerProduct.md] multiplies each
quadrature contribution by that factor.  For a function whose value is the parameter itself, such as
the [NearestReporterCoordinatesFunction.md] used by the single-parameter examples, the factor is 1.
Here it is $p_k^0$.

That decides which gradient a given number is: the gradient vector postprocessors emit
$\partial J / \partial \hat{p}_k$, which is $p_k^0 \, \partial J / \partial p_k$.

The same scale applies when reading results.  Recovered values, and the gold file, are in normalized
units; multiplying each by its $p_k^0$ gives the physical parameter.

What is measured here is that the unscaled form of this same problem made no progress under the
settings used.  That is a statement about this problem and those settings, not a general claim that
an inversion must be normalized to converge.

### Parameter Types Are Inferred From Names id=sec:typetrap

!alert warning title=A NEML2 parameter with no matching Function is silently miswired
[!param](/NEML2/parameter_types) must be present and the same length as
[!param](/NEML2/parameters), or the action raises a hard error.  Its *value*, however, is ignored.
The MOOSE-side type is always inferred from the parameter *name*: the action checks for a scalar
variable, then a `Function`, then a variable of that name, and falls back to `MATERIAL` when none
match.

Each NEML2 parameter driven by the optimizer therefore needs a MOOSE [Functions] object named
exactly the NEML2 parameter name.  Miss one and it infers as a material property, the optimizer
never reaches it, and nothing complains.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=Functions
         id=functions
         caption=One Function per driven NEML2 parameter, named to match.

A correct setup is visible in the transfer summary printed at the start of the run, which lists one
line per parameter:

```
MOOSE --> NEML2
  - elasticity_E (FUNCTION --> Scalar)
  - yield_sy (FUNCTION --> Scalar)
  - Xrate_C (FUNCTION --> Scalar)
  - flow_rate_eta (FUNCTION --> Scalar)
```

A parameter that reads `MATERIAL` there is miswired.  The summary can be inspected without running
the simulation using `--parse-neml2-only`.

### Gradient Assembly id=sec:assembly

One [AdjointStrainSymmetricStressGradInnerProduct.md] per parameter contracts the adjoint strain
against that parameter's stress derivative.  Each reads its
[!param](/VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct/stress_derivative_name)
from the corresponding NEML2-declared property and its
[!param](/VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct/function) from the
matching `Function`.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=VectorPostprocessors
         id=vpps
         caption=One inner product per inverted parameter.

The misfit enters the adjoint system as a point source through a [ReporterPointSource.md] driven by
the [OptimizationData.md] reporter, exactly as in the single-parameter examples.  A
[ConstantReporter.md] holds the current parameter values that the four `Function` objects read; the
driver overwrites it each optimization iteration.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=Reporters
         id=reporters
         caption=Measurement data and the current parameter values.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=DiracKernels
         id=dirac
         caption=The misfit is the adjoint source.

### The Optimization Driver id=sec:driver

[GeneralOptimization.md] splits TAO's single parameter vector into groups.  Each inverted parameter
is its own group of one value, named by
[!param](/OptimizationReporter/GeneralOptimization/parameter_names) and sized by
[!param](/OptimizationReporter/GeneralOptimization/num_values).  The four vector postprocessor
results are transferred back in the same order, so the concatenated gradient lines up with the
concatenated parameter vector.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/main.i
         id=main
         caption=The optimization driver.

Because the parameters are normalized, all four groups share the same initial condition and the same
bounds.  Uniform values follow from [!eqref](eq:normalization) rather than from the four parameters
happening to be comparable in size.

### Settings That Look Arbitrary id=sec:loadbearing

Three values in [main] and its sub-app look like ordinary tuning and are not.  What they share is
that changing any of them produces a failure that does not point back at the setting that caused it.

The lower bound on `eta` keeps the optimizer inside the domain where the material model is defined.
The Perzyna rate in [!eqref](eq:neml2_flow) evaluates $(f/\eta)^n$, so a trial step that takes $\eta$
to zero or below produces a NaN, and because the forward sub-app pins its minimum time step that NaN
becomes a hard failure rather than a time step cut.  The symptom is a solver crash partway through an
optimization, not a bounds error.

`-tao_ls_type` is deliberately unset, where the elasticity inversion in
`invOpt_elasticity_modular` sets it to `unit`.  A unit line search accepts the full step without
rescaling it, which suits a problem whose gradient is already a sensible step size.  In physical
units this one's was not, and the unscaled form of this problem made no progress under `unit`; that
is the measured comparison, and the historical reason the input is shaped this way.  Since
[#sec:normalization] the conditioning is no longer the obstacle it was, but the default line search
is what is measured here.  The symptom of the wrong choice is an optimizer that terminates while the
parameters have barely moved.

`-tao_gatol 1e-12` is the criterion that actually binds, and it is the one most likely to be loosened
by a reader who takes it for excess caution.  At convergence the four gradient columns in the gold
file sit near $10^{-13}$, below the $10^{-10}$ absolute-zero cutoff that [CSVDiff.md] applies, so
they compare as exact zero.  Loosen `-tao_gatol` and those columns rise above the cutoff, where they
are compared by relative error instead; two machines stopping at slightly different gradient norms
then disagree by hundreds of percent.  The symptom is an intermittent, platform-dependent test
failure that looks like a physics bug.

## Gradient Derivation id=sec:gradient

The objective is the least-squares displacement misfit,

\begin{equation}\label{eq:neml2_obj}
J(\mathbf{p}) = \frac{1}{2} \sum_{i=1}^{N} \left( u_h(\mathbf{x}_i; \mathbf{p}) - u^{*}_i \right)^2,
\end{equation}

where $u_h(\mathbf{x}_i; \mathbf{p})$ is the simulated displacement at measurement point
$\mathbf{x}_i$, $u^{*}_i$ is the measured value supplied through [OptimizationData.md], and
$\mathbf{p} = (E, \sigma_y, C, \eta)$.

The forward problem is the discrete equilibrium residual $\mathbf{R}(\mathbf{u}, \mathbf{p}) = 0$.
Differentiating $J$ subject to that constraint, and defining the adjoint state $\boldsymbol{\lambda}$
as the solution of

\begin{equation}\label{eq:neml2_adjoint}
\left( \frac{\partial \mathbf{R}}{\partial \mathbf{u}} \right)^{\top} \boldsymbol{\lambda}
= \frac{\partial J}{\partial \mathbf{u}},
\end{equation}

removes the unknown sensitivity $\partial \mathbf{u} / \partial p_k$ and leaves

\begin{equation}\label{eq:neml2_grad_R}
\frac{\partial J}{\partial p_k}
= - \boldsymbol{\lambda}^{\top} \frac{\partial \mathbf{R}}{\partial p_k}.
\end{equation}

The parameters enter the residual only through the stress, so
$\partial \mathbf{R} / \partial p_k$ is the virtual work of
$\partial \boldsymbol{\sigma} / \partial p_k$ and

\begin{equation}\label{eq:neml2_grad}
\frac{\partial J}{\partial p_k}
= - \int_{\Omega} \boldsymbol{\varepsilon}(\boldsymbol{\lambda}) : \frac{\partial \boldsymbol{\sigma}}{\partial p_k} \, d\Omega,
\end{equation}

where $\boldsymbol{\varepsilon}(\boldsymbol{\lambda})$ is the adjoint strain and
$\partial \boldsymbol{\sigma} / \partial p_k$ is the NEML2-supplied derivative property.  Each vector
postprocessor in [vpps] evaluates this integral and then multiplies it by the scale factor of
[!eqref](eq:normalization), so what it reports is $\partial J / \partial \hat{p}_k$; see
[#sec:normalization].

The operator in [!eqref](eq:neml2_adjoint) is the transpose of the forward Jacobian, that is, the
transposed consistent tangent.  The viscoplastic consistent tangent is not symmetric, so the
transpose is not the tangent itself.  A hand-written adjoint input file built by transcribing the
forward kernels would silently use the untransposed operator and produce a wrong gradient.  This
example therefore uses [TransientAndAdjoint.md], which assembles the adjoint operator by
transposing the forward Jacobian it already computes.  The alternative is the two-sub-app pattern of
[materialInv_ConstK.md], where a separate adjoint input file states the adjoint problem explicitly.
That pattern is a good fit where the adjoint operator is self-adjoint and easy to write down by
hand, and a poor one here, where it is a non-symmetric consistent tangent.

## Exactness at a Single Step id=sec:exactness

With [!param](/Executioner/TransientAndAdjoint/num_steps) set to 1, the gradient is exact to machine
precision.  Four facts combine to make that true.

1. `TransientAndAdjoint` loops backward over one fewer entry than the number of cached forward
   times.  A single forward step caches the initial condition and one solution, so exactly one
   backward pass runs.
2. The adjoint time vector that carries $\boldsymbol{\lambda}_{n+1}$ into step $n$ is
   zero-initialized and is written only *after* a converged adjoint solve.  The single pass
   therefore reads zero, which is the correct terminal condition
   $\boldsymbol{\lambda}_{N+1} = \mathbf{0}$.
3. State advancement runs exactly once, promoting the virgin zero state to "old".  Those old values
   are parameter-independent constants, not functions of $\mathbf{p}$.
4. The NEML2 tensors frozen during the backward sweep are exactly the state being adjointed.

Fact 3 is the load-bearing one.  The stateful quantities the material update reads from the previous
step, $\boldsymbol{\varepsilon}^p$ and $\mathbf{X}$, are identically zero and stay zero under
differentiation.  The total derivative of the stress with respect to a parameter then equals the
partial derivative NEML2 computes by reverse-mode differentiation of the return map, with no missing
history term.  Both halves of [!eqref](eq:neml2_grad) are exact, so their product is too.

## Limitation: More Than One Step Is Silently Wrong id=sec:multistep

!alert error title=Multi-step transient adjoints of a stateful material return a wrong gradient
For a material with parameter-dependent history, the multi-step transient adjoint does not error.
It returns a plausible, wrong gradient.

Setting [!param](/Executioner/TransientAndAdjoint/num_steps) to 2 breaks facts 1 and 4 in
[#sec:exactness].  From the second step onward the old state is itself a function of $\mathbf{p}$,
so the true sensitivity picks up a history term,

\begin{equation}\label{eq:missing_history}
\frac{d \boldsymbol{\sigma}_n}{d p_k}
= \frac{\partial \boldsymbol{\sigma}_n}{\partial p_k}
+ \frac{\partial \boldsymbol{\sigma}_n}{\partial \mathbf{h}_{n-1}} : \frac{d \mathbf{h}_{n-1}}{d p_k},
\end{equation}

where $\mathbf{h}$ collects the stateful quantities.  NEML2 supplies the first term only.  The
second is never formed and never propagated backward, and nothing in the assembly detects that it is
missing.  The optimizer converges to the wrong parameters, or fails to converge, with no diagnostic.

Keeping the forward solve to one step is a real restriction: it limits this workflow to inversions
whose measurements can be explained by a single load increment from a virgin state.  Multi-step
inversion of a stateful material needs the history sensitivity to be carried across steps, which the
current transient adjoint does not do.  The Limitations section of [TransientAndAdjoint.md] does not
presently list history dependence among its restrictions.

## The Adjoint Displacements Carry No Physics id=sec:adjointstrain

The adjoint variables have no kernels, no `Physics` block and no boundary conditions.  Their only
supporting object is a bare strain material that supplies the adjoint strain
[!eqref](eq:neml2_grad) contracts against.

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint.i
         block=Materials
         id=materials
         caption=A bare strain material on the adjoint displacements, with no kernels behind it.

!alert warning title=A Physics block on the adjoint displacements corrupts the adjoint source
`AdjointSolve` assembles the adjoint operator as the transposed *forward* Jacobian, and it assembles
the adjoint system's residual as a *source*.  Objects placed on the adjoint variables therefore
contribute to the right-hand side, while their Jacobian contribution is discarded.  A `Physics`
block on the adjoint displacements would inject stress divergence into the adjoint source and
silently change the answer.

The subtle part is why running the example cannot catch this.  At a single backward pass the adjoint
solution is still its zero initial condition when the source is assembled, so a stress-divergence
term evaluated on a zero field contributes nothing.  The miswired input produces the *correct*
gradient by coincidence.  It diverges only once a second backward pass reads a nonzero
$\boldsymbol{\lambda}$, which is a configuration [#sec:multistep] already rules out.  Only reading
the assembly reveals the error.

This is the same failure mode as the convention trap in [#sec:conventions]: the error hides in
precisely the regime a casual check does not exercise.

## Measurements Come From Outside MOOSE id=sec:truth

Synthetic measurements produced by the same MOOSE model that is later fitted constitute an inverse
crime: the discretization error cancels, and the recovered parameters look better than the method
warrants.

The measurements here are not produced by MOOSE at all.  Each block's homogeneous state reduces
the constitutive update to two scalar equations, derived below, and `measurements.csv` holds their
solutions at the true parameter values.  No MOOSE object, executable or output file takes part.
MOOSE only ever reads that file.

| Parameter | True value | Starting guess |
| - | - | - |
| `elasticity_E` | $10^{5}$ | $8 \times 10^{4}$ |
| `yield_sy` | $100$ | $85$ |
| `Xrate_C` | $1.2 \times 10^{4}$ | $8 \times 10^{3}$ |
| `flow_rate_eta` | $500$ | $400$ |

`elasticity_nu` ($0.3$), `Xrate_g` ($20$) and `flow_rate_n` ($2$) are held at their known values
throughout, the applied tractions are $t_b \in \{40, 80, 120, 160, 200\}$, and the single step
spans $\Delta t = 1$.  The truth is therefore a set of independently computed numbers, not the
output of an earlier MOOSE run.  The inversion is judged by comparing the recovered parameters
against the true column above.  Nothing in the comparison is self-referential.

The disconnected-block geometry is what makes this possible.  Each block is in a homogeneous
uniaxial-stress state, $\sigma_{zz} = t_b$ with all other components zero, so its displacement field
is exactly

\begin{equation}\label{eq:homogeneous}
u_x = \varepsilon_{xx}\, x, \qquad
u_y = \varepsilon_{yy}\, y, \qquad
u_z = \varepsilon_{zz}\, (z - 3b).
\end{equation}

The truth for a block is then not a PDE solve but a scalar one.  Under uniaxial stress the flow
direction is the unit deviator with $N_{zz} = \sqrt{2/3}$, and one backward-Euler step from the
virgin state gives a backstress of magnitude $\tfrac{2}{3} C \gamma / (1 + g \gamma)$, so the full
tensor update collapses to

\begin{equation}\label{eq:truth_gamma}
\gamma = \left\langle \frac{\sqrt{2/3}\,\bigl(t_b - \sigma_y - \sqrt{2/3}\, C \gamma / (1 + g \gamma)\bigr)}{\eta} \right\rangle^{n},
\end{equation}

\begin{equation}\label{eq:truth_strain}
\varepsilon_{zz} = \frac{t_b}{E} + \sqrt{2/3}\, \gamma,
\end{equation}

with $\gamma$ the plastic strain increment norm of [#sec:conventions] and $\langle\cdot\rangle$
the Macaulay bracket.  Every value in `measurements.csv` satisfies these two equations at the true
parameters, which makes the data checkable by substitution rather than by trusting a generator:

- The elastic blocks are exact by inspection: $t_b/E$ gives $4.0 \times 10^{-4}$ and
  $8.0 \times 10^{-4}$ for $t_b = 40, 80$ -- the committed values to the last digit.
- For a plastic block, recover $\gamma$ from the committed strain via [!eqref](eq:truth_strain)
  and substitute it into [!eqref](eq:truth_gamma).  At $t_b = 120$ the committed
  $\varepsilon_{zz} = 1.6602334 \times 10^{-3}$ gives
  $\gamma = (1.6602334 \times 10^{-3} - 1.2 \times 10^{-3}) / \sqrt{2/3} = 5.6367 \times 10^{-4}$,
  and the right-hand side of [!eqref](eq:truth_gamma) evaluates to
  $\left[0.8165 \times (120 - 100 - 5.4612) / 500\right]^{2} = 5.6367 \times 10^{-4}$ --
  the same number at the precision carried here.  The remaining blocks check the same way.

Anyone can regenerate the file from [!eqref](eq:truth_gamma) and [!eqref](eq:truth_strain) with
any scalar root finder; the committed values solve them to a residual below $10^{-9}$.

Differentiating [!eqref](eq:truth_strain) with respect to the four parameters gives the scaled
sensitivity matrix, $p_k\, \partial u_z / \partial p_k$.  For the shipped load levels it has
rank 4 (condition number $\approx 96$), which is the condition for the four parameters to be
identifiable from these measurements: the two elastic blocks constrain $E$ alone, and the three
plastic levels separate $\sigma_y$, $C$ and $\eta$ because they enter [!eqref](eq:truth_gamma)
with different dependencies on $\gamma$.

## Results id=sec:results

### Gradient Verification id=sec:gradcheck

The adjoint gradient is checked against central finite differences of the objective.  Agreement
across all four parameters, including the three that only the plastic blocks constrain, is what
demonstrates that [!eqref](eq:neml2_grad) and the single-step argument in [#sec:exactness] hold.

The table below is evaluated at the starting guess $E = 8 \times 10^4$, $\sigma_y = 85$,
$C = 8 \times 10^3$, $\eta = 400$, where the objective is $7.739892 \times 10^{-5}$.  The finite
differences are central, taken at a relative step of $10^{-6}$.

| Parameter | Adjoint $\partial J / \partial p_k$ | Finite difference | Relative difference |
| :- | :- | :- | :- |
| `elasticity_E` | -1.082977694194e-09 | -1.082977668792e-09 | 2.35e-08 |
| `yield_sy` | -4.237790927766e-06 | -4.237790947057e-06 | 4.55e-09 |
| `Xrate_C` | -2.621685078170e-08 | -2.621685081264e-08 | 1.18e-09 |
| `flow_rate_eta` | -4.842964146598e-07 | -4.842964149894e-07 | 6.81e-10 |

All four are negative, as expected from a guess that sits below the truth in every parameter.  The
worst agreement is $2.35 \times 10^{-8}$ relative, on $E$.

These physical components span a factor of about 3900, from $10^{-9}$ on $E$ to $10^{-6}$ on
$\sigma_y$.  That spread is the conditioning problem [#sec:normalization] exists to remove; it is a
property of the physical parametrization, not of the example as it ships.

In the normalized variables TAO actually steps, the same comparison at the same point reads

| Parameter | Adjoint $\partial J / \partial \hat{p}_k$ | Finite difference | Relative difference |
| :- | :- | :- | :- |
| `elasticity_E` | -8.663821553548e-05 | -8.663821399803e-05 | 1.77e-08 |
| `yield_sy` | -3.602122288601e-04 | -3.602122304998e-04 | 4.55e-09 |
| `Xrate_C` | -2.097348062536e-04 | -2.097348065011e-04 | 1.18e-09 |
| `flow_rate_eta` | -1.937185658639e-04 | -1.937185654943e-04 | 1.91e-09 |

Those components span a factor of about 4, against about 3900 physically.  That is the conditioning
change stated as a measurement.

The regression test measures the same agreement differently, and the two numbers are not
interchangeable.  [TaoGradientTester.md] scrapes a max-norm
$\lVert G - G_{\mathrm{fd}} \rVert / \lVert G \rVert$ over the whole gradient vector, using TAO's
own default `-tao_fd_delta` rather than the relative step used for the tables, and reports
$3.91 \times 10^{-7}$ against the test's $10^{-4}$ tolerance.  A vector max-norm is dominated by
whichever component carries the worst finite-difference truncation, which is $E$ in both
parametrizations, so it sits about an order of magnitude above the worst per-parameter figure.  The
tables are the per-parameter evidence; the scraped norm is what CI enforces.  The
`-tao_test_gradient` options described in [debuggingHelp.md] drive the same comparison from inside
the optimizer.

Normalizing improved that scraped norm from $3.55 \times 10^{-5}$ to $3.91 \times 10^{-7}$, two
orders, through conditioning alone.  The physics, the model and the evaluation point are unchanged,
and the physical gradients in the first table are unchanged with them.

### Parameter Recovery id=sec:convergence

The optimizer starts from $\hat{\mathbf{p}} = \mathbf{1}$, which is the physical guess
$E = 8 \times 10^4$, $\sigma_y = 85$, $C = 8 \times 10^3$, $\eta = 400$ under
[!eqref](eq:normalization).  Every parameter therefore starts below its true value.  Convergence
takes about 23 iterations, and the criterion that stops it is `-tao_gatol`, as described in
[#sec:loadbearing].

The recovered parameters, read from the gold file and multiplied by their scales, are

| Parameter | Recovered $\hat{p}_k$ | $p_k^0$ | Recovered physical | True | Relative error |
| :- | :- | :- | :- | :- | :- |
| `elasticity_E` | 1.2499999038574 | 8.0e4 | 99999.992 | 1e5 | 7.7e-08 |
| `yield_sy` | 1.1764704806029 | 85 | 99.999991 | 100 | 9.1e-08 |
| `Xrate_C` | 1.4999990218005 | 8.0e3 | 11999.992 | 1.2e4 | 6.5e-07 |
| `flow_rate_eta` | 1.2500011813110 | 400 | 500.00047 | 500 | 9.4e-07 |

All four are recovered to better than $10^{-6}$ relative.  $C$ and $\eta$ are the least accurate, by
about an order; both enter only through the plastic response, and $\eta$ only through its rate
dependence, so the measurements constrain them less directly than $E$ or $\sigma_y$.

The two parameters are worth checking by hand against [!eqref](eq:normalization): $1.25 \times 8
\times 10^4$ recovers $10^5$, and $1.5 \times 8 \times 10^3$ recovers $1.2 \times 10^4$.  The gold
file stores the normalized column, so a reader comparing it directly against the true values in
[#sec:truth] would otherwise find them off by exactly the scale factors.

No per-iteration convergence figure is published.  The gold file records the converged state, a
single row, rather than the iteration history, so there is no committed data from which to plot
parameter value against iteration.  Producing one would require committing the `OptimizationInfo`
output as a second gold file, which is not advisable: quasi-Newton iterates vary with BLAS version,
MPI decomposition and compiler, and unlike the converged gradients they are large enough to be
compared under relative error rather than absorbed by the absolute-zero cutoff.  That is the same
platform-dependent failure described in [#sec:loadbearing].
