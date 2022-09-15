# PorousFlowBasicTHM action with coupling_type = Hydro (no thermal or
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
    initial_condition = 1e7
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 293
  []
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  temperature = temperature
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = simple_fluid
  multiply_by_density = true
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
    thermal_expansion = 0
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    biot_coefficient = 1
    fluid_bulk_modulus = 2e9
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0   0 1e-13 0   0 0 1e-13'
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
    petsc_options_value = 'bcgs bjacobi 1e-10 1e-10 10000 30'
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
