# PorousFlowFullySaturated action with coupling_type = ThermoHydro (no
# mechanical effects), plus a Peaceman borehole with use_mobility = true
# to test that nodal relative permeability is added by this action.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [porepressure]
    initial_condition = 1E7
  []
  [temperature]
    initial_condition = 323.15
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = porepressure
  temperature = temperature
  dictator_name = dictator
  stabilization = none
  fp = simple_fluid
  gravity = '0 0 0'
[]

[BCs]
  [temperature]
    type = DirichletBC
    variable = temperature
    boundary = 'left right'
    value = 323.15
  []
[]

[UserObjects]
  [borehole_total_outflow_mass]
    type = PorousFlowSumQuantity
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    viscosity = 1e-3
    density0 = 1000
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.25
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0   0 1e-13 0   0 0 1e-13'
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
    wet_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 850
    density = 2700
  []
[]

[DiracKernels]
  [bh]
    type = PorousFlowPeacemanBorehole
    variable = porepressure
    SumQuantityUO = borehole_total_outflow_mass
    point_file = borehole.bh
    function_of = pressure
    fluid_phase = 0
    bottom_p_or_t = 0
    unit_weight = '0 0 0'
    use_mobility = true
    character = 1
  []
[]

[Postprocessors]
  [bh_report]
    type = PorousFlowPlotQuantity
    uo = borehole_total_outflow_mass
  []
[]

[Preconditioning]
  [usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.5
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
