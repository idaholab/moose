[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmin = -.6
    xmax = .6
  []
[]

[GlobalParams]
  advected_interp_method = 'average'
[]

[Variables]
  [fv_rho]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 2
  []
  [fv_vel]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 2
  []
[]

[FVKernels]
  [adv_rho]
    type = FVMatAdvection
    variable = fv_rho
    vel = 'fv_velocity'
  []

  [diff_rho]
    type = FVDiffusion
    variable = fv_rho
    coeff = coeff
  []

  [forcing_rho]
    type = FVBodyForce
    variable = fv_rho
    function = 'forcing_rho'
  []

  [adv_rho_u]
    type = FVMatAdvection
    variable = fv_vel
    vel = 'fv_velocity'
    advected_quantity = 'rho_u'
  []

  [diff_vel]
    type = FVDiffusion
    variable = fv_vel
    coeff = coeff
  []


  [forcing_vel]
    type = FVBodyForce
    variable = fv_vel
    function = 'forcing_vel'
  []
[]

[FVBCs]
  [boundary_rho]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    function = 'exact_rho'
    variable = fv_rho
  []
  [boundary_vel]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    function = 'exact_vel'
    variable = fv_vel
  []
[]

[Materials]
  [euler_material]
    type = ADCoupledVelocityMaterial
    vel_x = fv_vel
    rho = fv_rho
    velocity = 'fv_velocity'
  []
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  csv = true
[]

[Functions]
  [forcing_rho]
    type = ParsedFunction
    expression = '-1.331*sin(1.1*x)^2 + 1.331*sin(1.1*x) + 1.331*cos(1.1*x)^2'
  []
  [exact_rho]
    type = ParsedFunction
    expression = '1.1*sin(1.1*x)'
  []
  [forcing_vel]
    type = ParsedFunction
    expression = '-2.9282*sin(1.1*x)^2*cos(1.1*x) + 1.4641*cos(1.1*x)^3 + 1.331*cos(1.1*x)'
  []
  [exact_vel]
    type = ParsedFunction
    expression = '1.1*cos(1.1*x)'
  []
[]

[Postprocessors]
  [./l2_rho]
    type = ElementL2Error
    variable = fv_rho
    function = exact_rho
    execute_on = timestep_end
  [../]
  [./l2_vel]
    type = ElementL2Error
    variable = fv_vel
    function = exact_vel
    execute_on = timestep_end
  [../]
  [h]
    type = AverageElementSize
    execute_on = timestep_end
  []
[]
