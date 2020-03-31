# Multiphase models

Material objects that internally derive from `DerivativeFunctionMaterialBase`
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classDerivativeFunctionMaterialBase.html)),
like the materials for the [Parsed Function Kernels](FunctionMaterialKernels.md) are used to provide
the free energy expressions for each phase.

The flexible +multiphase model+ uses _n_ order parameters to control *n* phases while employing a
Lagrange multiplier based constraint to enforce the sum of all phase contributions to be one at every
point in the simulation cell.

For multiphase models with *n* phases [`DerivativeMultiPhaseMaterial`](DerivativeMultiPhaseMaterial.md)
can be used to form the global free energy as

\begin{equation}
F = \left[ \sum_i^n h_i(\eta_i)F_i \right] + g(\vec\eta)
\end{equation}

We need to enforce the constraint $k(\vec\eta)=0$ for

\begin{equation}
k(\vec \eta)=\left[\sum_i h(\eta_i)\right] - 1 \underbrace{- \frac\epsilon2\lambda}_{\text{Jacobian fill}},
\end{equation}

which ensures that the total weight of all phase free energy contributions at
each point in the simulation volume is exactly unity (up to an $\epsilon$). This
is achieved using either a hard or soft constraint enforcement method.

Check out the example input at `moose/modules/phase_field/examples/multiphase/DerivativeMultiPhaseMaterial.i` to see it in action.

## Lagrange multiplier constraint

As first (hard) method for constraint enforcement the Lagrange multiplier technique
is available, where the Lagrange multiplier $\lambda$ is a non-linear variable

With $a_i(\vec\eta,\vec c,v)$ being the weak form (Allen-Cahn) residual for the
$i$th non-conserved order parameter, we need to find $(\vec\eta,\lambda)$ satisfying
the boundary conditions and such that

\begin{equation}
\begin{aligned}
a_i(\vec\eta,\vec c,v) + \underbrace{\int_\Omega\lambda\frac{\partial k}{\partial\eta_i} v\,dx}_{L_1(\eta_i)} &=& 0 \\
\underbrace{\int_\Omega q\frac{\partial(\lambda k)}{\partial\lambda}\,dx}_{L_2(\lambda)} &=& 0
\end{aligned}
\end{equation}

holds for every test function $v$ and $q$.

The $L_1$ Lagrange residuals are provided by 
[`SwitchingFunctionConstraintEta`](/SwitchingFunctionConstraintEta.md) 
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classSwitchingFunctionConstraintEta.html)) 
kernels - one for each phase order parameter.

The $L_2$ Lagrange residual is provided by a 
[`SwitchingFunctionConstraintLagrange`](/SwitchingFunctionConstraintLagrange.md)
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classSwitchingFunctionConstraintLagrange.html))
kernel.

The *Jacobian fill* term introduces a small $\lambda$ dependence in the constraint
through a small $\epsilon$ factor (defaults to $10^{-9}$), which results in an
on-diagonal Jacobian value of $\epsilon$ in the $L_2$ kernel (it drops out in
the $L_1$ kernel). This is necessary to force a Jacobian matrix with *full rank*,
avoids "Zero pivot" PETSc-Errors, and greatly improves convergence. The cost is
a *violation* of the constraint by about $\epsilon$, however this constraint
violation can be made as small as the convergence limits.

## Penalty constraint

As an alternative (softer) constraint enforcement we provide the
[`SwitchingFunctionPenalty`](/SwitchingFunctionPenalty.md)
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classSwitchingFunctionPenalty.html))
kernel, which effectively adds a free energy penalty of $\gamma k(\vec \eta)^2$
(with $\epsilon=0$), where $\gamma$ is the penalty prefactor (`penalty`). The
constraint is enforced approximately to a tolerance of $\frac1\gamma$ (depending
on the shape and units of the free energy).

The gradient interface energy term for multiphase models with $n>2$ is derived
[here](ACMultiInterface.md) and provided by the [`ACMultiInterface`](ACMultiInterface.md) kernel.

## Example

An example material block looks like this (materials for phase field mobilities omitted for clarity).

```puppet
[Materials]
# Free energy for phase A

[./free_energy_A]
  type = DerivativeParsedMaterial
  block = 0
  f_name = Fa
  args = 'c'
  function = '(c-0.1)^2'
  third_derivatives = false
  enable_jit = true
[../]

# Free energy for phase B

[./free_energy_B]
  type = DerivativeParsedMaterial
  f_name = Fb
  args = 'c'
  function = '(c-0.9)^2'
  third_derivatives = false
  enable_jit = true
[../]

[./switching]
  type = SwitchingFunctionMaterial
  eta = eta
  h_order = SIMPLE
[../]

[./barrier]
  type = BarrierFunctionMaterial
  eta = eta
  g_order = SIMPLE
[../]

# Total free energy F = h(phi)*Fb + (1-h(phi))*Fa

[./free_energy]
  type = DerivativeTwoPhaseMaterial
  f_name = F    # Name of the global free energy function (use this in the Parsed Function Kernels)
  fa_name = Fa  # f_name of the phase A free energy function
  fb_name = Fb  # f_name of the phase B free energy function
  args = 'c'
  eta = eta     # order parameter that switches between A and B phase
  third_derivatives = false
  outputs = exodus
[../]
[]
```

!alert note
The phase free energies are single wells. The global free energy landscape
will however have a double well character in this example.
