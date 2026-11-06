# Exercise 6: gradient magnitude AuxKernel and volume fraction reducer on the Exercise 5 problem
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
  [grad_u]
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
  [grad_u]
    type = KokkosGradientMagnitudeAux
    variable = grad_u
    v = u
  []
[]

[Postprocessors]
  [amplitude]
    type = FunctionValuePostprocessor
    function = pulse
    execute_on = 'initial timestep_begin'
  []
  [grad_u_max]
    type = KokkosElementExtremeValue
    variable = grad_u
  []
  [hot_fraction]
    type = KokkosVolumeFractionAbove
    variable = u
    threshold = 0.5
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
