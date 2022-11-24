# Check error when using PorousFlowFullySaturated action,
# attempting to use both brine and single-component fluids

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  block = '0'
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = pp
  temperature = temp
  mass_fraction_vars = "nacl"
  fluid_properties_type = PorousFlowBrine
  nacl_name = nacl
  fp = simple_fluid
  dictator_name = dictator
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Variables]
  [pp]
    initial_condition = 20E6
  []
  [temp]
    initial_condition = 323.15
  []
  [nacl]
    initial_condition = 0.1047
  []
[]

[Kernels]
  # All provided by PorousFlowFullySaturated action
[]

[BCs]
  [t_bdy]
    type = DirichletBC
    variable = temp
    boundary = 'left right'
    value = 323.15
  []
  [p_bdy]
    type = DirichletBC
    variable = pp
    boundary = 'left right'
    value = 20E6
  []
  [nacl_bdy]
    type = DirichletBC
    variable = nacl
    boundary = 'left right'
    value = 0.1047
  []
[]

[Materials]
  # Thermal conductivity
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
    wet_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
  []

  # Specific heat capacity
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 850
    density = 2700
  []

  # Permeability
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-13 0 0  0 1E-13 0  0 0 1E-13'
  []

  # Porosity
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.3
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  file_base = fullsat_brine_except2
[]
