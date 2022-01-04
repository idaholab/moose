[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 4
  xmax = 2
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [sink]
    type = MooseVariableFVReal
  []
  [diffusive_flux_x]
    type = MooseVariableFVReal
  []
  [diffusive_flux_y]
    type = MooseVariableFVReal
  []
  [diffusive_flux_magnitude]
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
    variable = u
    coeff = 1.1
  []
  [sink]
    type = FVFunctorElementalKernel
    variable = u
    functor_name = 'sink_mat'
  []
[]

[FVBCs]
  [bounds]
    type = FVDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Materials]
  [functor_properties]
    type = ADGenericFunctorMaterial
    prop_names = 'sink_mat diffusive_coef'
    prop_values = 'sink 4.5'
  []
  [gradient_of_u]
    type = ADGenericFunctorGradientMaterial
    prop_names = 'grad_u'
    prop_values = 'u'
  []
[]

# Compute the diffusive flux magnitude
[AuxKernels]
  [diffusive_flux_x]
    type = ADFunctorVectorElementalAux
    variable = 'diffusive_flux_x'
    functor = 'grad_u'
    factor = 'diffusive_coef'
    component = 0
  []
  [diffusive_flux_y]
    type = ADFunctorVectorElementalAux
    variable = 'diffusive_flux_y'
    functor = 'grad_u'
    factor = 'diffusive_coef'
    component = 1
  []
  [diffusive_flux_magnitude]
    type = VectorMagnitudeAux
    variable = 'diffusive_flux_magnitude'
    x = 'diffusive_flux_x'
    y = 'diffusive_flux_y'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
