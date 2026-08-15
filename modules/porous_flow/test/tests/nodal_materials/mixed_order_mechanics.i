# SECOND-order displacements alongside a FIRST-order porepressure on a HEX27.
#
# This is the Taylor-Hood-style pairing that the nodal degree-of-freedom count
# exists to support.  The displacements are exempt from the requirement that
# everything read at the nodes share an FE type, because no Material reads them
# at the nodes: PorousFlowVolumetricStrain is quadpoint-only and rejects
# at_nodes = true.  Only the flow variables are read at the nodes, and here there
# is just the one.  Contrast mixed_order_flow_variables.i, where two *flow*
# variables of different order are correctly rejected.
#
# The mass time derivative is mass-lumped, so nodal Materials really are built
# and indexed here.  The flux is the non-upwinded fully-saturated kernel rather
# than PorousFlowAdvectiveFlux, to keep this test independent of the separate
# upwinding fix for the same issue.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 2
  []
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
  []
  [porepressure]
    order = FIRST
    initial_condition = 0.1
  []
[]

[BCs]
  [confinex]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  []
  [confiney]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  []
  [basefixed]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = back
  []
  [topdrained]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = front
  []
  [topload]
    type = NeumannBC
    variable = disp_z
    value = -1
    boundary = front
  []
[]

[Kernels]
  [grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
  [grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  []
  [poro_x]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.6
    variable = disp_z
    component = 2
  []
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
  [poro_vol_exp]
    type = PorousFlowMassVolumetricExpansion
    variable = porepressure
    fluid_component = 0
  []
  [flux]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = porepressure
    fluid_component = 0
    gravity = '0 0 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.8
    alpha = 1e-5
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 8
    density0 = 1
    thermal_expansion = 0
    viscosity = 0.96
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = porepressure
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.5 0 0   0 1.5 0   0 0 1.5'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0 # unimportant in this fully-saturated situation
    phase = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    solve_type = Newton
  []
[]

[Postprocessors]
  # The fluid mass integrates the nodal fluid density, porosity and saturation,
  # so it changes if the nodal Material values are wrong.  zdisp samples the
  # second-order displacement solution that shares the element with them
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [pp_base]
    type = PointValue
    variable = porepressure
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [zdisp]
    type = PointValue
    variable = disp_z
    point = '0 0 1'
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  csv = true
[]
