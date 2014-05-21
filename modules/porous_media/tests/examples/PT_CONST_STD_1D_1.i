[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  xmax = 10
[]

[Variables]
  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = 10e6
  [../]
  [./temperature]
    order = FIRST
    family = LAGRANGE
    initial_condition = 473.15
  [../]
[]

[Kernels]
  [./p_td]
    type = MassFluxTimeDerivative_PT
    variable = pressure
  [../]
  [./p_wmfp]
    type = WaterMassFluxPressure_PT
    variable = pressure
  [../]
  [./t_td]
    type = TemperatureTimeDerivative
    variable = temperature
  [../]
  [./t_d]
    type = TemperatureDiffusion
    variable = temperature
  [../]
  [./t_c]
    type = TemperatureConvection
    variable = temperature
  [../]
[]

[BCs]
  [./left_p]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 10e6
  [../]
  [./left_t]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 373.15
  [../]
  [./right_p]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 9.9e6
  [../]
  [./right_t]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 473.15
  [../]
[]

[Materials]
  [./GeothermalMaterial]
    block = 0
    solid_mechanics = false
    heat_transport = true
    fluid_flow = true
    chemical_reactions = false

    pressure = pressure
    temperature = temperature

    temp_dependent_fluid_props = false

    gravity = 0.0
    gx = 0.0
    gy = 0.0
    gz = 1.0

    porosity = 0.2
    permeability = 1.0e-13
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 5000.0

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-6
  [./Quadrature]
    type = Trap
  [../]
[]

[Outputs]
  file_base = PT_CONST_STD_1D_1_out
  output_initial = true
  interval = 10
  exodus = true
    [./console]
   type = Console
   perf_log = true
   linear_residuals = true
  [../]
[]
