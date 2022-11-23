# PorousFlowOutflowBC: testing Jacobian for single-phase, multi-component, with heat
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 2 3'
[]

[Variables]
  [pp]
    initial_condition = -1
  []
  [frac]
    initial_condition = 0.4
  []
  [T]
  []
[]

[PorousFlowUnsaturated]
  coupling_type = ThermoHydro
  add_darcy_aux = false
  fp = simple_fluid
  mass_fraction_vars = frac
  porepressure = pp
  temperature = T
  van_genuchten_alpha = 1
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1.2
    cp = 0.9
    cv = 1.1
    viscosity = 0.4
    thermal_expansion = 0.7
  []
[]

[BCs]
  [outflow0]
    type = PorousFlowOutflowBC
    boundary = 'front back top bottom'
    variable = frac
    mass_fraction_component = 0
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
  []
  [outflow1]
    type = PorousFlowOutflowBC
    boundary = 'left right top bottom'
    variable = pp
    mass_fraction_component = 1
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
  []
  [outflowT]
    type = PorousFlowOutflowBC
    boundary = 'left right top bottom'
    flux_type = heat
    variable = T
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.4
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.1 0.2 0.3 1.8 0.9 1.7 0.4 0.3 1.1'
  []
  [matrix_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 0.5
    specific_heat_capacity = 2.2E-3
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1.1 1.2 1.3 0.8 0.9 0.7 0.4 0.3 0.1'
    wet_thermal_conductivity = '0.1 0.2 0.3 1.8 1.9 1.7 1.4 1.3 1.1'
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
  solve_type = NEWTON
  dt = 1E-7
  num_steps = 1
#  petsc_options = '-snes_test_jacobian -snes_force_iteration'
#  petsc_options_iname = '-snes_type --ksp_type -pc_type -snes_convergence_test'
#  petsc_options_value = ' ksponly     preonly   none     skip'
[]
