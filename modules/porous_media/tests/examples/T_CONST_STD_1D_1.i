# This is a simple single-phase, non-isothermal, heat diffusion problem on a 100mx10x1m column
# Temperature is initially 200 C.  A BC of 100 C is applied to the left-hand side of the column
# The standard GeothermalMaterial is used, with constant values of density/viscosity.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  xmax = 10
[]

[Variables]
  [./temperature]       # Main temperature variable declared
    order = FIRST
    family = LAGRANGE
    initial_condition = 473.15 # [K]
  [../]
[]

[AuxVariables]
  [./viscosity_water]   # This aux variables are not necessary to run the problem, it just display the viscosity
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./density_water]     # This aux variables are not necessary to run the problem, it just display the density
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./t_td]          # Heat time derivative kernel for PT (single-phase) problems
    type = TemperatureTimeDerivative
    variable = temperature
  [../]
  [./t_d]           # Heat diffusion kernel for PT (single-phase) problems
    type = TemperatureDiffusion
    variable = temperature
  [../]
[]

[AuxKernels]
  [./density_water]     # This aux kernels are not necessary to run the problem, they just display the density
    type = MaterialRealAux
    variable = density_water
    property = density_water
  [../]
  [./viscosity_water]   # This aux kernels are not necessary to run the problem, they just display the viscosity
    type = MaterialRealAux
    variable = viscosity_water
    property = viscosity_water
  [../]
[]

[BCs]
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
    fluid_flow                  = true      # H - flaged true to provide fluid props even though this is not an H problem
    solid_mechanics             = false     # M
    chemical_reactions          = false     # C

    # couple in main NL variable
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

    # material property inputs from FluidFlow
    temp_dependent_fluid_props  = false     # we want to have consant density and viscosity in this problem
    constant_density            = 1000      # [kg/m^2]
    constant_viscosity          = 0.12e-3   # [Pa.s]

  [../]
[]

[Executioner]
  type = Transient
  num_steps = 50
  dt = 10000.0

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-6
[]

[Outputs]
  file_base = T_CONST_STD_1D_1_out
  output_initial = true
  interval = 10
  exodus = true
  console = true
[]
