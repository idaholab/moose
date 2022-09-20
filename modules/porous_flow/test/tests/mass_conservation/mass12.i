# The sample is an annulus in RZ coordinates.
# Roller BCs are applied to the r_min, r_max and bottom boundaries
# A constant displacement is applied to the top: disp_z = -0.01*t.
# There is no fluid flow.
# Fluid mass conservation is checked.
#
# Under these conditions
# fluid_mass = volume0 * rho0 * exp(P0/bulk) = pi*3 * 1 * exp(0.1/0.5) = 11.51145
# volume0 * rho0 * exp(P0/bulk) = volume * rho0 * exp(P/bulk), so
# P - P0 = bulk * log(volume0 / volume) = 0.5 * log(1 / (1 - 0.01*t))

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 1.0
  xmax = 2.0
  ymin = -0.5
  ymax = 0.5
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
  [porepressure]
    initial_condition = 0.1
  []
[]

[BCs]
  [bottom_roller]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = bottom
  []
  [side_rollers]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = 'left right'
  []
  [top_move]
    type = FunctionDirichletBC
    variable = disp_z
    function = -0.01*t
    boundary = top
  []
[]

[Kernels]
  [grad_stress_r]
    type = StressDivergenceRZTensors
    variable = disp_r
    component = 0
  []
  [grad_stress_z]
    type = StressDivergenceRZTensors
    variable = disp_z
    component = 1
  []
  [poro_r]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    variable = disp_r
    component = 0
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    variable = disp_z
    component = 1
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

[AuxVariables]
  [stress_rr]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_rz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_tt]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_rr]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rr
    index_i = 0
    index_j = 0
  []
  [stress_rz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rz
    index_i = 0
    index_j = 1
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 1
    index_j = 1
  []
  [stress_tt]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_tt
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
    type = ComputeAxisymmetricRZSmallStrain
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
    porous_flow_vars = 'porepressure disp_r disp_z'
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
    point = '1 0 0'
    variable = porepressure
  []
  [rdisp]
    type = PointValue
    outputs = csv
    point = '2 0 0'
    use_displaced_mesh = false
    variable = disp_r
  []
  [stress_rr]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_rr
  []
  [stress_zz]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_zz
  []
  [stress_tt]
    type = PointValue
    outputs = csv
    point = '1 0 0'
    variable = stress_tt
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
