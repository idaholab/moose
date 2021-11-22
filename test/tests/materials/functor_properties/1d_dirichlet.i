[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 2
[]

[Variables]
  [v]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [sink]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [sink]
    type = FunctionIC
    variable = sink
    function = 'x^3'
  []
[]


[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
  [sink]
    type = FVFunctorElementalKernel
    variable = v
    functor_name = 'sink_mat'
  []
[]

[FVBCs]
  [bounds]
    type = FVDirichletBC
    variable = v
    boundary = 'left right'
    value = 0
  []
[]

[Materials]
  active = 'functor'
  [functor]
    type = ADGenericFunctorMaterial
    prop_names = sink_mat
    prop_values = sink
  []
  [overlapping_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'sink_mat'
    prop_values = v
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
