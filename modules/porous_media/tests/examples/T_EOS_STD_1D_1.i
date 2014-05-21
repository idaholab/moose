# This is a simple single-phase, non-isothermal, heat advection-diffusion problem on a 100mx10x1m column
# Temperature is initially 200 C.  A BC of 100 C is applied to the left-hand side of the column
# The standard GeothermalMaterial is used, with the water/steam EOS routine coupled in to determine variable
# density/viscosity.
# This is the same problem as T_CONST_STD_1D_1.i, but with variable fluid properties

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  xmax = 10
[]

[Variables]
# Main pressure and temperature variables declared
# This is an constant pressure problem (heat transfer only), but pressure is needed for calculation of fluid props
# which are both temp and pressure dependent

  [./pressure]
    order = FIRST
    family = LAGRANGE
    initial_condition = 10e6   # [Pa]
  [../]
  [./temperature]
    order = FIRST
    family = LAGRANGE
    initial_condition = 473.15 # [K]
  [../]
[]

[AuxVariables]
  [./v_x]           # This aux variables are not necessary to run the problem, it just display the velocity
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscosity_water]# This aux variables are not necessary to run the problem, it just display the viscosity
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./density_water] # This aux variables are not necessary to run the problem, it just display the density
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./p_wmfp]        # Mass diffusion kernel for PT (single-phase) problems
    type = WaterMassFluxPressure_PT
    variable = pressure
  [../]
  [./t_td]          # Heat time derivative kernel for PT (single-phase) problems
    type = TemperatureTimeDerivative
    variable = temperature
  [../]
  [./t_d]           # Heat diffusion kernel for PT (single-phase) problems
    type = TemperatureDiffusion
    variable = temperature
  [../]
  [./t_c]           # Heat convection kernel for PT (single-phase) problems
    type = TemperatureConvection
    variable = temperature
  [../]
[]

[AuxKernels]
# Thess aux kernels are not necessary to run the problem, they just display the velocity/density/viscosity, which change due to
# temperature effects

  [./vx]            # This aux variables are not necessary to run the problem, it just display the velocity
    type = VelocityAux
    variable = v_x
    component = 0
  [../]
  [./density_water] # This aux variables are not necessary to run the problem, it just display the viscosity
    type = MaterialRealAux
    variable = density_water
    property = density_water
  [../]
  [./viscosity_water]# This aux variables are not necessary to run the problem, it just display the density
    type = MaterialRealAux
    variable = viscosity_water
    property = viscosity_water
  [../]
[]

[BCs]
  [./const_p]
    type = DirichletBC
    variable = pressure
    boundary = 'left right'
    value = 10e6
  [../]
  [./left_t]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 373.15
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

    # flag booleans to define THMC problem
    heat_transport              = true      # T
    fluid_flow                  = true      # H - flaged true to provide fluid props to pressure diffusion kernel even though this is not a H problem
    solid_mechanics             = false     # M
    chemical_reactions          = false     # C

    # couple in main NL variables
    pressure                    = pressure
    temperature                 = temperature

    # material property inputs from PorousMedia (base class - parameters availible to all THMC materials)
    gravity                     = 0.0       # gravity magnitude [m/s^2]
    gx                          = 0.0       # x-component of gravity vector
    gy                          = 0.0       # y-component of gravity vector
    gz                          = 1.0       # z-component of gravity vector
    porosity                    = 0.2
    permeability                = 1.0e-13   # [m^2]

    # material property inputs from HeatTransport
    specific_heat_water         = 4186      # [J/(kg.K)]
    specific_heat_rock          = 920       # [J/(kg.K)]
    thermal_conductivity        = 2.5       # [W/(kg.K)]

    # material property inputs from FluidFlow (must have temp_dependent_fluid_props = true and temperature coupled in to get varaible density/viscosity)
    temp_dependent_fluid_props  = true      # we want to have variable density and viscosity in this problem, so we use the water/steam EOS routine
    water_steam_properties      = water_steam_properties    # coupling of WaterSteamEOS UserObject below to use for calculation of fluid props

  [../]
[]

[UserObjects]
  [./water_steam_properties]    # This user object contains the water and steam equations of state that are used to calculate variable fluid props
    type = WaterSteamEOS
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 10000.0

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-6
[]

[Outputs]
  file_base = T_EOS_STD_1D_1_out
  output_initial = true
  interval = 10
  exodus = true
  console = true
[]
