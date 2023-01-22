diff=1.1
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
    nx = 64
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
  [./diffusion]
    type = FVDiffusion
    variable = v
    coeff = coeff
  [../]
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [boundary]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    function = 'exact'
    variable = v
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '${diff}'
  []
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
    expression = '3*x^2 + 2*x + 1'
  []
  [forcing]
    type = ParsedFunction
    expression = '-${diff}*6 + ${a} * (6*x + 2)'
    # expression = '-${diff}*6'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
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
