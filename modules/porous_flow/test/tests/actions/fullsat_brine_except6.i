# Error checking: attempt to use non-standard time_unit with PorousFlowBrine

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
  porepressure = pp
  temperature = 273.15
  mass_fraction_vars = nacl
  fluid_properties_type = PorousFlowBrine
  nacl_name = nacl
  time_unit = days
  dictator_name = dictator
  stabilization = none
[]

[Variables]
  [pp]
    initial_condition = 20E6
  []
  [nacl]
    initial_condition = 0.1047
  []
[]

[Materials]
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
