# FluidFlower International Benchmark study model
# CSIRO 2023
#
# This example can be used to reproduce the results presented by the
# CSIRO team as part of this benchmark study. See
# Green, C., Jackson, S.J., Gunning, J., Wilkins, A. and Ennis-King, J.,
# 2023. Modelling the FluidFlower: Insights from Characterisation and
# Numerical Predictions. Transport in Porous Media.
#
# This example takes a long time to run! The large density contrast
# between the gas phase CO2 and the water makes convergence very hard,
# so small timesteps must be taken during injection.
#
# This example uses a simplified mesh in order to be run during the
# automated testing. To reproduce the results of the benchmark study,
# replace the simple layered input mesh with the one located in the
# large_media submodule.
#
# The mesh file contains:
# - porosity as given by FluidFlower description
# - permeability as given by FluidFlower description
# - subdomain ids for each sand type
#
# The nominal thickness of the FluidFlower tank is 19mm. To keep masses consistent
# with the experiment, porosity and permeability are multiplied by the thickness
thickness = 0.019
#
# Properties associated with each sand type associated with mesh block ids
#
# block 0 - ESF (very fine sand)
sandESF = '0 10 20'
sandESF_pe = 1471.5
sandESF_krg = 0.09
sandESF_swi = 0.32
sandESF_krw = 0.71
sandESF_sgi = 0.14

# block 1 - C - Coarse lower
sandC = '1 21'
sandC_pe = 294.3
sandC_krg = 0.05
sandC_swi = 0.14
sandC_krw = 0.93
sandC_sgi = 0.1

# block 2 - D - Coarse upper
sandD = '2 22'
sandD_pe = 98.1
sandD_krg = 0.02
sandD_swi = 0.12
sandD_krw = 0.95
sandD_sgi = 0.08

# block 3 - E - Very Coarse lower
sandE = '3 13 23'
sandE_pe = 10
sandE_krg = 0.1
sandE_swi = 0.12
sandE_krw = 0.93
sandE_sgi = 0.06

# block 4 - F - Very Coarse upper
sandF = '4 14 24 34'
sandF_pe = 10
sandF_krg = 0.11
sandF_swi = 0.12
sandF_krw = 0.72
sandF_sgi = 0.13

# block 5 - G - Flush Zone
sandG = '5 15 35'
sandG_pe = 10
sandG_krg = 0.16
sandG_swi = 0.1
sandG_krw = 0.75
sandG_sgi = 0.06

# block 6 - Fault 1 - Heterogeneous
fault1 = '6 26'
fault1_pe = 10
fault1_krg = 0.16
fault1_swi = 0.1
fault1_krw = 0.75
fault1_sgi = 0.06

# block 7 - Fault 2 - Impermeable
# Note: this fault has been removed from the mesh (no elements in this region)

# block 8 - Fault 3 - Homogeneous
fault3 = '8'
fault3_pe = 10
fault3_krg = 0.16
fault3_swi = 0.1
fault3_krw = 0.75
fault3_sgi = 0.06

# Top layer
top_layer = '9'

# Boxes A, B an C used to report values (sg, sgr, xco2, etc)
boxA = '10 13 14 15 34 35'
boxB = '20 21 22 23 24 26'
boxC = '34 35'

# Furthermore, the seal sand unit in boxes A and B
seal_boxA = '10'
seal_boxB = '20'

# CO2 injection details:
# CO2 density ~1.8389 kg/m3 at 293.15 K, 1.01325e5 Pa
# Injection in Port (9, 3) for 5 hours.
# Injection in Port (17, 7) for 2:45 hours.
# Injection of 10 ml/min = 0.1666 ml/s = 1.666e-7 m3/s = ~3.06e-7 kg/s.
# Total mass of CO2 injected ~ 8.5g.
inj_rate = 3.06e-7

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = 'fluidflower_test.e'
    # file = '../../../../large_media/porous_flow/examples/fluidflower/fluidflower.e'
    use_for_exodus_restart = true
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -9.81 0'
  temperature = temperature
  log_extension = false
[]

[Variables]
  [pgas]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [z]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    scaling = 1e4
  []
[]

[AuxVariables]
  [xnacl]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.0055
  []
  [temperature]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 20
  []
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_from_file_var = porosity
  []
  [porosity_times_thickness]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [permeability]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_from_file_var = permeability
  []
  [permeability_times_thickness]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [saturation_water]
    family = MONOMIAL
    order = CONSTANT
  []
  [saturation_gas]
    family = MONOMIAL
    order = CONSTANT
  []
  [pressure_water]
    family = MONOMIAL
    order = CONSTANT
  []
  [pc]
    family = MONOMIAL
    order = CONSTANT
  []
  [x0_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [x0_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_gas]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [porosity_times_thickness]
    type = ParsedAux
    variable = porosity_times_thickness
    coupled_variables = porosity
    expression = 'porosity * ${thickness}'
    execute_on = 'initial'
  []
  [permeability_times_thickness]
    type = ParsedAux
    variable = permeability_times_thickness
    coupled_variables = permeability
    expression = 'permeability * ${thickness}'
    execute_on = 'initial'
  []
  [pressure_water]
    type = ADPorousFlowPropertyAux
    variable = pressure_water
    property = pressure
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [saturation_water]
    type = ADPorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [saturation_gas]
    type = ADPorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [density_water]
    type = ADPorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [density_gas]
    type = ADPorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [x1_water]
    type = ADPorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
  [x1_gas]
    type = ADPorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
  [x0_water]
    type = ADPorousFlowPropertyAux
    variable = x0_water
    property = mass_fraction
    phase = 0
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [x0_gas]
    type = ADPorousFlowPropertyAux
    variable = x0_gas
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [pc]
    type = ADPorousFlowPropertyAux
    variable = pc
    property = capillary_pressure
    execute_on = 'initial timestep_end'
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    variable = pgas
    fluid_component = 0
  []
  [flux0]
    type = FVPorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  []
  [diff0]
    type = FVPorousFlowDispersiveFlux
    variable = pgas
    fluid_component = 0
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    variable = z
    fluid_component = 1
  []
  [flux1]
    type = FVPorousFlowAdvectiveFlux
    variable = z
    fluid_component = 1
  []
  [diff1]
    type = FVPorousFlowDispersiveFlux
    variable = z
    fluid_component = 1
    disp_long = '0 0'
    disp_trans = '0 0'
  []
[]

[DiracKernels]
  [injector1]
    type = ConstantPointSource
    point = '0.9 0.3 0'
    value = ${inj_rate}
    variable = z
  []
  [injector2]
    type = ConstantPointSource
    point = '1.7 0.7 0'
    value = ${inj_rate}
    variable = z
  []
[]

[Controls]
  [injection1]
    type = ConditionalFunctionEnableControl
    enable_objects = 'DiracKernels::injector1'
    conditional_function = injection_schedule1
  []
  [injection2]
    type = ConditionalFunctionEnableControl
    enable_objects = 'DiracKernels::injector2'
    conditional_function = injection_schedule2
  []
[]

[Functions]
  [initial_p]
    type = ParsedFunction
    symbol_names = 'p0 g H rho0'
    symbol_values = '101.325e3 9.81 1.5 1002'
    expression = 'p0 + rho0 * g * (H - y)'
  []
  [injection_schedule1]
    type = ParsedFunction
    expression = 'if(t >= 0 & t <= 1.8e4, 1, 0)'
  []
  [injection_schedule2]
    type = ParsedFunction
    expression = 'if(t >= 8.1e3 & t <= 1.8e4, 1, 0)'
  []
[]

[ICs]
  [p]
    type = FunctionIC
    variable = pgas
    function = initial_p
  []
[]

[FVBCs]
  [pressure_top]
    type = FVPorousFlowAdvectiveFluxBC
    boundary = top
    porepressure_value = 1.01325e5
    variable = pgas
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
  [watertab]
    type = TabulatedBicubicFluidProperties
    fp = water
    save_file = false
    pressure_min = 1e5
    pressure_max = 1e6
    temperature_min = 290
    temperature_max = 300
    num_p = 20
    num_T = 10
  []
  [co2]
    type = CO2FluidProperties
  []
  [co2tab]
    type = TabulatedBicubicFluidProperties
    fp = co2
    save_file = false
    pressure_min = 1e5
    pressure_max = 1e6
    temperature_min = 290
    temperature_max = 300
    num_p = 20
    num_T = 10
  []
  [brine]
    type = BrineFluidProperties
    water_fp = watertab
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [sandESF_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandESF_pe}
    lambda = 2
    block = ${sandESF}
    pc_max = 1e4
    sat_lr = ${sandESF_swi}
  []
  [sandC_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandC_pe}
    lambda = 2
    block = ${sandC}
    pc_max = 1e4
    sat_lr = ${sandC_swi}
  []
  [sandD_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandD_pe}
    lambda = 2
    block = ${sandD}
    pc_max = 1e4
    sat_lr = ${sandD_swi}
  []
  [sandE_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandE_pe}
    lambda = 2
    block = ${sandE}
    pc_max = 1e4
    sat_lr = ${sandE_swi}
  []
  [sandF_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandF_pe}
    lambda = 2
    block = ${sandF}
    pc_max = 1e4
    sat_lr = ${sandF_swi}
  []
  [sandG_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${sandG_pe}
    lambda = 2
    block = ${sandG}
    pc_max = 1e4
    sat_lr = ${sandG_swi}
  []
  [fault1_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${fault1_pe}
    lambda = 2
    block = ${fault1}
    pc_max = 1e4
    sat_lr = ${fault1_swi}
  []
  [fault3_pc]
    type = PorousFlowCapillaryPressureBC
    pe = ${fault3_pe}
    lambda = 2
    block = ${fault3}
    pc_max = 1e4
    sat_lr = ${fault3_swi}
  []
  [top_layer_pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
    block =  ${top_layer}
  []
  [sandESF_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandESF_pc
  []
  [sandC_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandC_pc
  []
  [sandD_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandD_pc
  []
  [sandE_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandE_pc
  []
  [sandF_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandF_pc
  []
  [sandG_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = sandG_pc
  []
  [fault1_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = fault1_pc
  []
  [fault3_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = fault3_pc
  []
  [top_layer_fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2tab
    capillary_pressure = top_layer_pc
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = temperature
  []
  [sandESF_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandESF_fs
    capillary_pressure = sandESF_pc
    block = ${sandESF}
  []
  [sandC_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandC_fs
    capillary_pressure = sandC_pc
    block = ${sandC}
  []
  [sandD_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandD_fs
    capillary_pressure = sandD_pc
    block = ${sandD}
  []
  [sandE_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandE_fs
    capillary_pressure = sandE_pc
    block = ${sandE}
  []
  [sandF_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandF_fs
    capillary_pressure = sandF_pc
    block = ${sandF}
  []
  [sandG_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = sandG_fs
    capillary_pressure = sandG_pc
    block = ${sandG}
  []
  [fault1_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = fault1_fs
    capillary_pressure = fault1_pc
    block = ${fault1}
  []
  [fault3_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = fault3_fs
    capillary_pressure = fault3_pc
    block = ${fault3}
  []
  [top_layer_brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    fluid_state = top_layer_fs
    capillary_pressure = top_layer_pc
    block = ${top_layer}
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = porosity_times_thickness
  []
  [permeability]
    type = ADPorousFlowPermeabilityConstFromVar
    perm_xx = permeability_times_thickness
    perm_yy = permeability_times_thickness
    perm_zz = permeability_times_thickness
  []
  [diffcoeff]
    type = ADPorousFlowDiffusivityConst
    tortuosity = '1 1'
    diffusion_coeff = '2e-9 2e-9 0 0'
  []
  [sandESF_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandESF_swi}
    sum_s_res = ${fparse sandESF_sgi + sandESF_swi}
    scaling = ${sandESF_krw}
    block = ${sandESF}
  []
  [sandESF_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandESF_sgi}
    sum_s_res = ${fparse sandESF_sgi + sandESF_swi}
    scaling = ${sandESF_krg}
    block = ${sandESF}
  []
  [sandC_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandC_swi}
    sum_s_res = ${fparse sandC_sgi + sandC_swi}
    scaling = ${sandC_krw}
    block = ${sandC}
  []
  [sandC_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandC_sgi}
    sum_s_res = ${fparse sandC_sgi + sandC_swi}
    scaling = ${sandC_krg}
    block = ${sandC}
  []
  [sandD_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandD_swi}
    sum_s_res = ${fparse sandD_sgi + sandD_swi}
    scaling = ${sandD_krw}
    block = ${sandD}
  []
  [sandD_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandD_sgi}
    sum_s_res = ${fparse sandD_sgi + sandD_swi}
    scaling = ${sandD_krg}
    block = ${sandD}
  []
  [sandE_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandE_swi}
    sum_s_res = ${fparse sandE_sgi + sandE_swi}
    scaling = ${sandE_krw}
    block = ${sandE}
  []
  [sandE_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandE_sgi}
    sum_s_res = ${fparse sandE_sgi + sandE_swi}
    scaling = ${sandE_krg}
    block = ${sandE}
  []
  [sandF_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandF_swi}
    sum_s_res = ${fparse sandF_sgi + sandF_swi}
    scaling = ${sandF_krw}
    block = ${sandF}
  []
  [sandF_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandF_sgi}
    sum_s_res = ${fparse sandF_sgi + sandF_swi}
    scaling = ${sandF_krg}
    block = ${sandF}
  []
  [sandG_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${sandG_swi}
    sum_s_res = ${fparse sandG_sgi + sandG_swi}
    scaling = ${sandG_krw}
    block = ${sandG}
  []
  [sandG_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${sandG_sgi}
    sum_s_res = ${fparse sandG_sgi + sandG_swi}
    scaling = ${sandG_krg}
    block = ${sandG}
  []
  [fault1_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${fault1_swi}
    sum_s_res = ${fparse fault1_sgi + fault1_swi}
    scaling = ${fault1_krw}
    block = ${fault1}
  []
  [fault1_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${fault1_sgi}
    sum_s_res = ${fparse fault1_sgi + fault1_swi}
    scaling = ${fault1_krg}
    block = ${fault1}
  []
  [fault3_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    s_res = ${fault3_swi}
    sum_s_res = ${fparse fault3_sgi + fault3_swi}
    scaling = ${fault3_krw}
    block = ${fault3}
  []
  [fault3_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    s_res = ${fault3_sgi}
    sum_s_res = ${fparse fault3_sgi + fault3_swi}
    scaling = ${fault3_krg}
    block = ${fault3}
  []
  [top_layer_relperm0]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 0
    lambda = 2
    block = ${top_layer}
  []
  [top_layer_relperm1]
    type = ADPorousFlowRelativePermeabilityBC
    phase = 1
    nw_phase = true
    lambda = 2
    block = ${top_layer}
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options = '-ksp_snes_ew'
    petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres lu mumps NONZERO'
    # petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type -sub_pc_type -sub_pc_factor_shift_type -sub_pc_factor_levels -ksp_gmres_restart'
    # petsc_options_value = 'gmres hypre boomeramg lu NONZERO 4 301'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dtmax = 60
  start_time = 0
  end_time = 4.32e5
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  nl_max_its = 15
  l_tol = 1e-5
  l_abs_tol = 1e-8
  # line_search = none # Can be a useful option for this problem
  [TimeSteppers]
    [time]
      type = FunctionDT
      growth_factor = 2
      cutback_factor_at_failure = 0.5
      function = 'if(t<1.8e4, 2, if(t<3.6e4, 20, 60))'
    []
  []
[]

[Postprocessors]
  [p_5_3]
    type = PointValue
    variable = pgas
    point = '0.5 0.3 0'
    execute_on = 'initial timestep_end'
  []
  [p_5_3_w]
    type = PointValue
    variable = pressure_water
    point = '0.5 0.3 0'
    execute_on = 'initial timestep_end'
  []
  [p_5_7]
    type = PointValue
    variable = pgas
    point = '0.5 0.7 0'
    execute_on = 'initial timestep_end'
  []
  [p_5_7_w]
    type = PointValue
    variable = pressure_water
    point = '0.5 0.7 0'
    execute_on = 'initial timestep_end'
  []
  [p_9_3]
    type = PointValue
    variable = pgas
    point = '0.9 0.3 0'
    execute_on = 'initial timestep_end'
  []
  [p_9_3_w]
    type = PointValue
    variable = pressure_water
    point = '0.9 0.3 0'
    execute_on = 'initial timestep_end'
  []
  [p_15_5]
    type = PointValue
    variable = pgas
    point = '1.5 0.5 0'
    execute_on = 'initial timestep_end'
  []
  [p_15_5_w]
    type = PointValue
    variable = pressure_water
    point = '1.5 0.5 0'
    execute_on = 'initial timestep_end'
  []
  [p_17_7]
    type = PointValue
    variable = pgas
    point = '1.7 0.7 0'
    execute_on = 'initial timestep_end'
  []
  [p_17_7_w]
    type = PointValue
    variable = pressure_water
    point = '1.7 0.7 0'
    execute_on = 'initial timestep_end'
  []
  [p_17_11]
    type = PointValue
    variable = pgas
    point = '1.7 1.1 0'
    execute_on = 'initial timestep_end'
  []
  [p_17_11_w]
    type = PointValue
    variable = pressure_water
    point = '1.7 1.1 0'
    execute_on = 'initial timestep_end'
  []
  [x0mass]
    type = FVPorousFlowFluidMass
    fluid_component = 0
    phase = '0 1'
    execute_on = 'initial timestep_end'
  []
  [x1mass]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    execute_on = 'initial timestep_end'
  []
  [x1gas]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '1'
    execute_on = 'initial timestep_end'
  []
  [boxA]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    block = ${boxA}
    execute_on = 'initial timestep_end'
  []
  [imm_A_sandESF]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandESF_sgi}
    block = 10
    execute_on = 'initial timestep_end'
  []
  [imm_A_sandE]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandE_sgi}
    block = 13
    execute_on = 'initial timestep_end'
  []
  [imm_A_sandF]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandF_sgi}
    block = '14 34'
    execute_on = 'initial timestep_end'
  []
  [imm_A_sandG]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandG_sgi}
    block = '15 35'
    execute_on = 'initial timestep_end'
  []
  [imm_A]
    type = LinearCombinationPostprocessor
    pp_names = 'imm_A_sandESF imm_A_sandE imm_A_sandF imm_A_sandG'
    pp_coefs = '1 1 1 1'
    execute_on = 'initial timestep_end'
  []
  [diss_A]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 0
    block = ${boxA}
    execute_on = 'initial timestep_end'
  []
  [seal_A]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    block = ${seal_boxA}
    execute_on = 'initial timestep_end'
  []
  [boxB]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    block = ${boxB}
    execute_on = 'initial timestep_end'
  []
  [imm_B_sandESF]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandESF_sgi}
    block = 20
    execute_on = 'initial timestep_end'
  []
  [imm_B_sandC]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandC_sgi}
    block = 21
    execute_on = 'initial timestep_end'
  []
  [imm_B_sandD]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandD_sgi}
    block = 22
    execute_on = 'initial timestep_end'
  []
  [imm_B_sandE]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandE_sgi}
    block = 23
    execute_on = 'initial timestep_end'
  []
  [imm_B_sandF]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${sandF_sgi}
    block = 24
    execute_on = 'initial timestep_end'
  []
  [imm_B_fault1]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = ${fault1_sgi}
    block = 26
    execute_on = 'initial timestep_end'
  []
  [imm_B]
    type = LinearCombinationPostprocessor
    pp_names = 'imm_B_sandESF imm_B_sandC imm_B_sandD imm_B_sandE imm_B_sandF imm_B_fault1'
    pp_coefs = '1 1 1 1 1 1'
    execute_on = 'initial timestep_end'
  []
  [diss_B]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = 0
    block = ${boxB}
    execute_on = 'initial timestep_end'
  []
  [seal_B]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    block = ${seal_boxB}
    execute_on = 'initial timestep_end'
  []
  [boxC]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0'
    block = ${boxC}
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  # exodus = true
  [csv]
     type = CSV
  []
[]
