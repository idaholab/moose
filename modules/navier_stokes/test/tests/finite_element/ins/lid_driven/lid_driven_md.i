[GlobalParams]
  gravity = '0 0 0'

  order = FIRST
  family = LAGRANGE

  u = vel_x
  v = vel_y
  pressure = p
  temperature = T
  porosity = porosity
  eos = eos
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.1
    ymin = 0
    ymax = 0.1
    nx = 20
    ny = 20
    elem_type = QUAD4
  []

  [middle_node]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = 'bottom_left_corner'
    coord = '0 0'
  []
[]

[FluidProperties]
  [./eos]
    type = SimpleFluidProperties
    density0 = 100              # kg/m^3
    thermal_expansion = 0       # K^{-1}
    cp =  1272.0
    viscosity = 0.1             # Pa-s, Re=rho*u*L/mu = 100*1*0.1/0.1 = 100
  [../]
[]

[Variables]
  # velocity
  [./vel_x]
    scaling = 1.e-1
    initial_condition = 0.0
  [../]
  [./vel_y]
    scaling = 1.e-1
    initial_condition = 0.0
  [../]
  # Pressure
  [./p]
    scaling = 1
    initial_condition = 1.0e5
  [../]
[]

[AuxVariables]
  [rho]
    # incompressible flow, rho = constant
    initial_condition = 100
  []
  [T]
    # nothing really depends on T, but eos requires temperature
    initial_condition = 800
  []
  [porosity]
    # nothing really depends on porosity, but PINSFEFluidPressureTimeDerivative requires it
    # need make it conditional
    initial_condition = 1
  []
[]

[Materials]
  [flow_mat]
    type = INSFEMaterial
  []
[]

[Kernels]
  # mass eqn
  [mass_time]
    type = PINSFEFluidPressureTimeDerivative
    variable = p
  []
  [mass_space]
    type = INSFEFluidMassKernel
    variable = p
  []

  # x-momentum eqn
  [x_momentum_time]
    type = PINSFEFluidVelocityTimeDerivative
    variable = vel_x
  []
  [x_momentum_space]
    type = INSFEFluidMomentumKernel
    variable = vel_x
    component = 0
  []

  # y-momentum eqn
  [y_momentum_time]
    type = PINSFEFluidVelocityTimeDerivative
    variable = vel_y
  []
  [y_momentum_space]
    type = INSFEFluidMomentumKernel
    variable = vel_y
    component = 1
  []
[]

[BCs]
  [x_zero]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom left right'
    value = 0
  []
  [x_lid]
    type = DirichletBC
    variable = vel_x
    boundary = 'top'
    value = 1
  []
  [y_zero]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom top left right'
    value = 0
  []

  [p_anchor]
    type = DirichletBC
    variable = p
    boundary = 'bottom_left_corner'
    value = 1e5
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]


[Executioner]
  type = Transient

  dt = 0.01
  dtmin = 1.e-4

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 100'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-6
  l_max_its = 100

  start_time = 0.0
  end_time = 2
  num_steps = 5
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = false
  interval = 1
  execute_on = 'initial timestep_end'
  [./console]
    type = Console
    output_linear = false
  [../]
  [./out]
    type = Exodus
    hide = 'porosity'
  [../]
[]
