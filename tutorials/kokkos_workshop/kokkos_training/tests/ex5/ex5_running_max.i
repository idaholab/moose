# Exercise 5 bonus: a stateful running maximum of u driven by a pulsed source
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [pulse]
    type = ParsedFunction
    expression = 'sin(pi * t)'
  []
[]

[Kernels]
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [source]
    type = KokkosBodyForce
    variable = u
    value = 10
    # A host postprocessor read by a Kokkos kernel
    postprocessor = amplitude
  []
[]

[BCs]
  [zero]
    type = KokkosDirichletBC
    variable = u
    boundary = 'left right'
    value = 0
  []
[]

[Materials]
  [max]
    type = KokkosRunningMax
    v = u
  []
[]

[AuxVariables]
  [u_max]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [u_max]
    type = KokkosMaterialRealAux
    variable = u_max
    property = running_max
  []
[]

[Postprocessors]
  [amplitude]
    type = FunctionValuePostprocessor
    function = pulse
    execute_on = 'initial timestep_begin'
  []
  [u_now]
    type = KokkosElementExtremeValue
    variable = u
  []
  # Maximum over elements of the element average of the running maximum, so it can trail the
  # pointwise maximum u_now slightly while u is still rising
  [u_peak]
    type = KokkosElementExtremeValue
    variable = u_max
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  end_time = 2
[]

[Outputs]
  csv = true
[]
