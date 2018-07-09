# Two-phase models

MOOSE provides capabilities that enable the easy development of multiphase field model. A free energy
expression has to be provided for each individual phase. Two different systems exist to combine those
*phase free energies* into a *global free energy*.

Material objects that internally derive from ```DerivativeFunctionMaterialBase```
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classDerivativeFunctionMaterialBase.html)),
like the materials for the [Parsed Function Kernels](ParsedFunctionKernels) are used to provide the
free energy expressions for each phase.

The simplified +two-phase model+ uses a single order parameter to switch between the two phases. A
global free energy is constructed using a meta material class that combines the phase free energies.

For two phase models the ```DerivativeTwoPhaseMaterial```
([Doxygen](http://mooseframework.org/docs/doxygen/modules/classDerivativeTwoPhaseMaterial.html)) can
be used to combine two phase free energies into a global free energy (which the Allen-Cahn and
Cahn-Hilliard kernels use to evolve the system) as

\begin{equation}
F = \left(1-h(\eta)\right) F_a + h(\eta)F_b + Wg(\eta)
\end{equation}

!syntax parameters /Materials/DerivativeTwoPhaseMaterial

Check out the example input at ```modules/phase_field/tests/MultiPhase/derivativetwophasematerial.i``` to see it in action.

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
  block = 0
  f_name = Fb
  args = 'c'
  function = '(c-0.9)^2'
  third_derivatives = false
  enable_jit = true
[../]

[./switching]
  type = SwitchingFunctionMaterial
  block = 0
  eta = eta
  h_order = SIMPLE
[../]

[./barrier]
  type = BarrierFunctionMaterial
  block = 0
  eta = eta
  g_order = SIMPLE
[../]

# Total free energy F = h(phi)*Fb + (1-h(phi))*Fa

[./free_energy]
  type = DerivativeTwoPhaseMaterial
  block = 0
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

Note that the phase free energies are single wells. The global free energy landscape will however
have a double well character in this example.
