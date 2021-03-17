[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Variables]
  # we have to impose non-zero initial conditions in order to avoid an initially
  # singular matrix
  [fv_vel]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 2
  []
  [fv_rho]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 2
  []
[]

[FVKernels]
  # del * rho * velocity * velocity
  [adv_rho_u]
    type = FVMatAdvection
    variable = fv_vel
    vel = 'fv_velocity'
    advected_quantity = 'rho_u'
  []

  # del * rho * velocity
  [adv_rho]
    type = FVMatAdvection
    variable = fv_rho
    vel = 'fv_velocity'
  []
[]


[FVBCs]
  [left_vel]
    type = FVDirichletBC
    variable = fv_vel
    value = 1
    boundary = 'left'
  []
  [left_rho]
    type = FVDirichletBC
    variable = fv_rho
    value = 1
    boundary = 'left'
  []

  # del * rho * velocity * velocity
  [right_vel]
    type = FVMatAdvectionOutflowBC
    variable = fv_vel
    vel = 'fv_velocity'
    advected_quantity = 'rho_u'
    boundary = 'right'
  []

  # del * rho * velocity
  [adv_rho]
    type = FVMatAdvectionOutflowBC
    variable = fv_rho
    vel = 'fv_velocity'
    boundary = 'right'
  []
[]

[Materials]
  [euler_material]
    type = ADCoupledVelocityMaterial
    vel_x = fv_vel
    rho = fv_rho
    velocity = 'fv_velocity'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = 'none'
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
