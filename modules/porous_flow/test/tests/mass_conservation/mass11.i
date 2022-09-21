# The sample is a single unit element, with roller BCs on the sides and bottom.
# The top is free to move and fluid is injected at a constant rate of 1kg/s
# There is no fluid flow.
# Fluid mass conservation is checked.
# Under these conditions the fluid mass should increase at 1kg/s
# The porepressure should increase: rho0 * exp(P/bulk) = rho * exp(P0/bulk) + 1*t
# The stress_zz should be exactly biot * P since total stress is zero
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
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
    biot_coefficient = 0.3
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    component = 2
    variable = disp_z
  []
  [poro_vol_exp]
    type = PorousFlowMassVolumetricExpansion
    variable = porepressure
    fluid_component = 0
  []
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = porepressure
  []
[]

[DiracKernels]
  [inject]
    type = PorousFlowPointSourceFromPostprocessor
    point = '0 0 0'
    mass_flux = 1.0
    variable = porepressure
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
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
    permeability = '0.5 0 0   0 0.5 0   0 0 0.5'
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
    m = 0.5
    alpha = 1
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '0 0 0'
    variable = porepressure
  []
  [zdisp]
    type = PointValue
    outputs = csv
    point = '0 0 0.5'
    use_displaced_mesh = false
    variable = disp_z
  []
  [stress_xx]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_xx
  []
  [stress_yy]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_yy
  []
  [stress_zz]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_zz
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-14 1E-8 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  dt = 2
[]

[Outputs]
  execute_on = 'initial timestep_end'
  [csv]
    type = CSV
  []
[]
