a=1.1
diff=1.1

[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[FVKernels]
  [advection]
    type = FVElementalAdvection
    variable = v
    velocity = '${a} ${fparse 2 * a} 0'
    advected_quantity = 'mat_u'
    grad_advected_quantity = 'mat_grad_u'
  []
  [reaction]
    type = FVReaction
    variable = v
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = ${diff}
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [diri]
    type = FVFunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact'
    variable = v
  []
[]

[Materials]
  [mat]
    type = ADCoupledGradientMaterial
    mat_prop = 'mat_u'
    grad_mat_prop = 'mat_grad_u'
    u = v
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(x)*cos(y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '-2*a*sin(x)*sin(y) + a*cos(x)*cos(y) + 2*diff*sin(x)*cos(y) + sin(x)*cos(y)'
    symbol_names = 'a diff'
    symbol_values = '${a} ${diff}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type -sub_pc_type'
  petsc_options_value = 'asm      NONZERO                   lu'
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
