# Fracture physics.  Heat is injected at the left end.  Heat advects along the fracture and conducts to the matrix App
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = 0
    xmax = 100.0
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [frac_P]
    initial_condition = 2 # MPa
  []
  [frac_T]
    initial_condition = 40 # degC
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = frac_P
  temperature = frac_T
  fp = simple_fluid
  stabilization = KT
  flux_limiter_type = minmod
  gravity = '0 0 0'
  pressure_unit = MPa
  temperature_unit = Celsius
  time_unit = seconds
[]

[Kernels]
  [toMatrix]
    type = PorousFlowHeatMassTransfer
    variable = frac_T
    v = transferred_matrix_T
    transfer_coefficient = 1E2
    save_in = joules_per_s
  []
[]

[Modules]
  [PorousFlow]
    [BCs]
      [left_injection]
        type = PorousFlowSinkBC
        boundary = left
        fluid_phase = 0
        T_in = 373 # Kelvin!
        fp = simple_fluid
        flux_function = -10 # 10 kg/s
      []
    []
  []
[]

[BCs]
  [mass_production]
    type = PorousFlowSink
    boundary = right
    variable = frac_P
    flux_function = 10
  []
  [heat_production]
    type = PorousFlowSink
    boundary = right
    variable = frac_T
    flux_function = 10
    fluid_phase = 0
    use_enthalpy = true
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1E9 # in Pa
    density0 = 1000
    thermal_expansion = 0 # for simplicity
    viscosity = 1E-3 # in Pa.s
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 1E-2 # includes fracture aperture of 1E-2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-8 0 0   0 1E-8 0   0 0 1E-8' # roughness times a^3/12
  []
  [internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 1
    specific_heat_capacity = 0 # basically no rock inside the fracture
  []
  [aq_thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.6E-2 0 0  0 0.6E-2 0  0 0 0.6E-2' # thermal conductivity of water times fracture aperture
  []
[]

[AuxVariables]
  [transferred_matrix_T]
  []
  [joules_per_s]
  []
[]

[VectorPostprocessors]
  [heat_transfer_rate]
    type = NodalValueSampler
    outputs = none
    sort_by = id
    variable = joules_per_s
  []
  [frac]
    type = NodalValueSampler
    outputs = frac
    sort_by = x
    variable = 'frac_T frac_P'
  []
[]

[Preconditioning]
  [entire_jacobian]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 100
  end_time = 100
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
  [frac]
    type = CSV
    execute_on = final
  []
[]
