a=1.1
diff=1.1

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  [../]
[]

[Problem]
  coord_type = 'RZ'
[]

[Variables]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  [../]
[]

[FVKernels]
  [./advection]
    type = FVElementalAdvection
    variable = v
    velocity = '${a} ${a} 0'
  [../]
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

[Functions]
[exact]
  type = ParsedFunction
  expression = 'sin(x)*cos(y)'
[]
[forcing]
  type = ParsedFunction
  expression = '-a*sin(x)*sin(y) + diff*sin(x)*cos(y) + sin(x)*cos(y) + (x*a*cos(x)*cos(y) + a*sin(x)*cos(y))/x - (-x*diff*sin(x)*cos(y) + diff*cos(x)*cos(y))/x'
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
  [./error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
