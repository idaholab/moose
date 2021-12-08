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
    functor_name = 'ad_sink'
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
  [converter_to_regular]
    type = FunctorADConverter
    ad_props_in = 'sink'
    reg_props_out = 'regular_sink_0'
  []
  # Just to change the name
  [functor]
    type = GenericFunctorMaterial
    prop_names = 'regular_sink_1'
    prop_values = 'regular_sink_0'
  []
  [converter_to_ad]
    type = FunctorADConverter
    reg_props_in = 'regular_sink_1'
    ad_props_out = 'ad_sink'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
