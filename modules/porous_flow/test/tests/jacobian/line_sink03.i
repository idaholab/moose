# PorousFlowPeacemanBorehole with 2-phase, 3-components, with enthalpy, internal_energy, and thermal_conductivity
# NOTE: this test has suffered from repeated failures since its inception.  The problem always appears to be caused by having too many Dirac points in an element: see #10471.  As of Nov2020, the dirac7 DiracKernel uses only one Dirac point, not ten_points.bh.  One day it would be good to be able to use point_file = ten_points.bh
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
  [ppwater]
  []
  [ppgas]
  []
  [massfrac_ph0_sp0]
  []
  [massfrac_ph0_sp1]
  []
  [massfrac_ph1_sp0]
  []
  [massfrac_ph1_sp1]
  []
  [temp]
  []
[]


[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp ppwater ppgas massfrac_ph0_sp0 massfrac_ph0_sp1 massfrac_ph1_sp0 massfrac_ph1_sp1'
    number_fluid_phases = 2
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
  [dummy_outflow0]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow1]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow2]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow3]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow4]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow5]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow6]
    type = PorousFlowSumQuantity
  []
  [dummy_outflow7]
    type = PorousFlowSumQuantity
  []
[]

[ICs]
  [temp]
    type = RandomIC
    variable = temp
    min = 1
    max = 2
  []
  [ppwater]
    type = RandomIC
    variable = ppwater
    min = -1
    max = 0
  []
  [ppgas]
    type = RandomIC
    variable = ppgas
    min = 0
    max = 1
  []
  [massfrac_ph0_sp0]
    type = RandomIC
    variable = massfrac_ph0_sp0
    min = 0
    max = 1
  []
  [massfrac_ph0_sp1]
    type = RandomIC
    variable = massfrac_ph0_sp1
    min = 0
    max = 1
  []
  [massfrac_ph1_sp0]
    type = RandomIC
    variable = massfrac_ph1_sp0
    min = 0
    max = 1
  []
  [massfrac_ph1_sp1]
    type = RandomIC
    variable = massfrac_ph1_sp1
    min = 0
    max = 1
  []
[]

[Kernels]
  [dummy_temp]
    type = TimeDerivative
    variable = temp
  []
  [dummy_ppwater]
    type = TimeDerivative
    variable = ppwater
  []
  [dummy_ppgas]
    type = TimeDerivative
    variable = ppgas
  []
  [dummy_m00]
    type = TimeDerivative
    variable = massfrac_ph0_sp0
  []
  [dummy_m01]
    type = TimeDerivative
    variable = massfrac_ph0_sp1
  []
  [dummy_m10]
    type = TimeDerivative
    variable = massfrac_ph1_sp0
  []
  [dummy_m11]
    type = TimeDerivative
    variable = massfrac_ph1_sp1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
    cv = 1.1
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 0.5
    thermal_expansion = 0
    viscosity = 1.4
    cv = 1.8
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph0_sp1 massfrac_ph1_sp0 massfrac_ph1_sp1'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.1 0.02 0.03 0.02 0.0 0.01 0.03 0.01 0.3'
  []
[]

[DiracKernels]
  #active = 'dirac6 dirac2' # incorrect jacobian for ny=2
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5' # correct jacobian for ny=2
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5 dirac6' # incorrect jacobian for ny=2
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5 dirac7' # correct jacobian in dbg, but not in opt, for ny=2
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5 dirac6' # incorrect jacobian in dbg, but correct for opt, for ny=1
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5' # correct jacobian, for ny=1
  #active = 'dirac0 dirac1 dirac2 dirac3 dirac4 dirac5 dirac6' # incorrect jacobian in dbg, but correct for opt, for ny=1.  row24, col 21 and 22 are wrong.  row24=node3, 21=ppwater, 22=ppgas, 24=massfrac_ph0_sp1 (all at node3)
  [dirac0]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 0
    variable = ppwater
    point_file = one_point.bh
    line_length = 1
    SumQuantityUO = dummy_outflow0
    character = 1
    bottom_p_or_t = -10
    unit_weight = '1 2 3'
    re_constant = 0.123
  []
  [dirac1]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 1
    variable = ppgas
    line_length = 1
    line_direction = '-1 -1 -1'
    use_relative_permeability = true
    point_file = one_point.bh
    SumQuantityUO = dummy_outflow1
    character = -0.5
    bottom_p_or_t = 10
    unit_weight = '1 2 -3'
    re_constant = 0.3
  []
  [dirac2]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 0
    variable = massfrac_ph0_sp0
    line_length = 1.3
    line_direction = '1 0 1'
    use_mobility = true
    point_file = one_point.bh
    SumQuantityUO = dummy_outflow2
    character = 0.6
    bottom_p_or_t = -4
    unit_weight = '-1 -2 -3'
    re_constant = 0.4
  []
  [dirac3]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 0
    variable = massfrac_ph0_sp1
    line_length = 1.3
    line_direction = '1 1 1'
    use_enthalpy = true
    mass_fraction_component = 0
    point_file = one_point.bh
    SumQuantityUO = dummy_outflow3
    character = -1
    bottom_p_or_t = 3
    unit_weight = '0.1 0.2 0.3'
    re_constant = 0.5
  []
  [dirac4]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 1
    variable = massfrac_ph1_sp0
    function_of = temperature
    line_length = 0.9
    line_direction = '1 1 1'
    mass_fraction_component = 1
    use_internal_energy = true
    point_file = one_point.bh
    SumQuantityUO = dummy_outflow4
    character = 1.1
    bottom_p_or_t = -7
    unit_weight = '-1 2 3'
    re_constant = 0.6
  []
  [dirac5]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 1
    variable = temp
    line_length = 0.9
    function_of = temperature
    line_direction = '1 2 3'
    mass_fraction_component = 2
    use_internal_energy = true
    point_file = one_point.bh
    SumQuantityUO = dummy_outflow5
    character = 0.9
    bottom_p_or_t = -8
    unit_weight = '1 2 1'
    re_constant = 0.7
  []
  [dirac6]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 0
    variable = ppwater
    point_file = nine_points.bh
    SumQuantityUO = dummy_outflow6
    character = 0
    bottom_p_or_t = 10
    unit_weight = '0.0 0.0 0.0'
  []
  [dirac7]
    type = PorousFlowPeacemanBorehole
    fluid_phase = 1
    variable = massfrac_ph0_sp0
    use_mobility = true
    mass_fraction_component = 1
    use_relative_permeability = true
    use_internal_energy = true
    point_file = one_point.bh
    #NOTE this commented-out line: point_file = ten_points.bh
    SumQuantityUO = dummy_outflow7
    character = -1
    bottom_p_or_t = 10
    unit_weight = '0.1 0.2 0.3'
  []
[]


[Preconditioning]
  [check]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  file_base = line_sink03
[]
