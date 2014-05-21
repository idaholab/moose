[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  xmin = -100
  xmax = 100
  ymin = -100
  ymax = 100
  zmin = -10
  zmax = 10
[]

[Variables]
  [./pressure]
    initial_condition = 0.10e6
  [../]
  [./temperature]
    initial_condition = 323.15
  [../]
[]

[AuxVariables]
 #----none of these aukernels are required for computation----#
 #----                                                    ----#
 #----velocity[x,y], density and viscosity of fluid       ----#
  [./v_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./density_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscosity_water]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./v_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
 #----Pressure: diffusion, source(injection at well), time derivative / Temp.: diffusion----#
  [./p_wmfp]
    type = WaterMassFluxPressure_PT
    variable = pressure
  [../]
  [./p_td]
    type = MassFluxTimeDerivative_PT
    variable = pressure
  [../]
  [./t_d]
    type = TemperatureDiffusion
    variable = temperature
  [../]
  [./water_sink]
    type = SourceSink
    variable = pressure
    value = '-1.0 -1.0 -1.0'     # kg/s rate
    point = '0.0 0.0 0.0'
    size = '20.0 20.0 20.0'
  [../]
[]

[AuxKernels]
 #----none of these aukernels are required for computation----#
 #----                                                    ----#
 #----velocity[x,y], density and viscosity of fluid       ----#
  [./vx]
    type = VelocityAux
    variable = v_x
    component = 0
  [../]
  [./density_water]
    type = MaterialRealAux
    variable = density_water
    property = density_water
  [../]
  [./viscosity_water]
    type = MaterialRealAux
    variable = viscosity_water
    property = viscosity_water
  [../]
  [./vy]
    type = VelocityAux
    variable = v_y
    component = 1
  [../]
[]

[BCs]
  [./left_p]
    type = DirichletBC
    variable = pressure
    boundary = '1 2 3 4'
    value = 0.10e6
  [../]
  [./temp]
    type = DirichletBC
    variable = temperature
    boundary = '1 2 3 4'
    value = 323.15
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

    water_steam_properties = water_steam_properties
    temp_dependent_fluid_props = true

    gravity = 0.0
    gx = 0.0
    gy = 0.0
    gz = 1.0

    porosity = 0.5
    permeability = 1.0e-12
  [../]
[]

[UserObjects]
  [./water_steam_properties]
    type = WaterSteamEOS
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1000
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -snes_ls -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 201 basic 0.7'
  nl_abs_tol = 1e-8
  line_search = basic

  [./Quadrature]
    type = Trap
  [../]
[]

[Outputs]
  file_base = PT_EOS_STD_2D_1_out
  output_initial = true
  exodus = true
  console = true
[]
