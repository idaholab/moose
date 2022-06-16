a=1.1

[GlobalParams]
  advected_interp_method = 'average'
[]

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -0.6
    xmax = 0.6
    nx = 2
  [../]
[]

[Variables]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  [./advection]
    type = FVAdvection
    variable = v
    velocity = '${a} 0 0'
  [../]
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [advection]
    type = FVAdvectionFunctionBC
    boundary = 'left right'
    exact_solution = 'exact'
    variable = v
    velocity = '${a} 0 0'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    value = '1.1 * sin(1.1 * x)'
  []
  [forcing]
    type = ParsedFunction
    value = '${a} * 1.1 * 1.1 * cos(1.1 * x)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      NONZERO'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console'    execute_on = 'timestep_end'
  [../]
  [h]
    type = AverageElementSize
    outputs = 'console'    execute_on = 'timestep_end'
  []
[]
