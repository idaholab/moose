# This is a simple single phase, isothermal, mass transport example problem on a 100mx10mx1m column.
# Pressure is initially 100 kPa. A BC of 110 kPa is applied to the left-hand side of the column, and
# is allowed diffused acrosss the domain until it reaches steady state. The standard GeothermalMaterial
# is used, with constant values of viscosity and density
# This is the same problem as P_EOS_STD_1D_1.i, but with constant fluid properties, so temperature
# is no longer needed as a main variable

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  xmax = 100
  ymax = 10
[]

[Variables]
  [./pressure]      # Main pressure variable declared
    initial_condition = 0.1e6 #<-- [Pa]
  [../]
[]

[AuxVariables]
  [./v_x]           # This aux variable is not necessary to run the problem, it just displays the velocity
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./p_wmfp]        # Mass diffusion kernel for PT (single-phase) problems
    type = WaterMassFluxPressure_PT
    variable = pressure
  [../]
[]

[AuxKernels]
  [./vx]            # This aux kernel is not necessary to run the problem, it just displays the velocity
    type = VelocityAux
    variable = v_x
    component = 0
  [../]
[]

[BCs]
  [./left_p]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 0.11e6
  [../]
  [./right_p]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0.1e6
  [../]
[]

[Materials]

  [./GeothermalMaterial]
    block = 0

    # flag booleans to define THMC problem
    heat_transport              = false     # T
    fluid_flow                  = true      # H
    solid_mechanics             = false     # M
    chemical_reactions          = false     # C

    # couple in main NL variable
    pressure                    = pressure

    # material property inputs from PorousMedia (base class - parameters availible to all THMC materials)
    gravity                     = 0.0       # gravity magnitude [m/s^2]
    gx                          = 0.0       # x-component of gravity vector
    gy                          = 0.0       # y-component of gravity vector
    gz                          = 1.0       # z-component of gravity vector
    porosity                    = 0.5
    permeability                = 1.0e-12   # [m^2]

    # material property inputs from FluidFlow
    temp_dependent_fluid_props  = false     # we want to have consant density and viscosity in this problem
    constant_density            = 1000      # [kg/m^2]
    constant_viscosity          = 0.12e-3   # [Pa.s]

  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = P_CONST_STD_1d_1_out
  output_initial = true
  exodus = true
  console = true
[]
