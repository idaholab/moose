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
    type = FVMatAdvection
    variable = v
    vel = 'fv_velocity'
  [../]
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [boundary]
    type = FVMatAdvectionFunctionBC
    boundary = 'left right'
    variable = v
    vel = 'fv_velocity'
    flux_variable_exact_solution = 'exact'
    vel_x_exact_solution = '${a}'
  []
[]

[Materials]
  [adv_material]
    type = ADCoupledVelocityMaterial
    vel_x = '${a}'
    rho = 'v'
    velocity = 'fv_velocity'
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
