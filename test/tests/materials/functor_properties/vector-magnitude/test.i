[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [mag]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = ADFunctorElementalAux
    variable = mag
    functor = mat_mag
  []
[]

[FVKernels]
  [v_diff]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
  [u_diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
[]

[FVBCs]
  [v_left]
    type = FVDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = FVDirichletBC
    variable = v
    boundary = 'right'
    value = 1
  []
  [u_bottom]
    type = FVDirichletBC
    variable = u
    boundary = 'bottom'
    value = 0
  []
  [u_top]
    type = FVDirichletBC
    variable = u
    boundary = 'top'
    value = 1
  []
[]

[Materials]
  [functor]
    type = ADVectorMagnitudeFunctorMaterial
    x_functor = u
    y_functor = v
    vector_magnitude_name = mat_mag
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
