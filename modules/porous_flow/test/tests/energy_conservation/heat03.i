# The sample is a single unit element, with roller BCs on the sides
# and bottom.  A constant displacement is applied to the top: disp_z = -0.01*t.
# There is no fluid flow or heat flow.
# Heat energy conservation is checked.
#
# Under these conditions (here L is the height of the sample: L=1 in this case):
# porepressure = porepressure(t=0) - (Fluid bulk modulus)*log(1 - 0.01*t)
# stress_xx = (bulk - 2*shear/3)*disp_z/L (remember this is effective stress)
# stress_zz = (bulk + 4*shear/3)*disp_z/L (remember this is effective stress)
# Also, the total heat energy must be conserved: this is
# fluid_mass * fluid_heat_cap * temperature + (1 - porosity) * rock_density * rock_heat_cap * temperature * volume
# Since fluid_mass is conserved, and volume = (1 - 0.01*t), this can be solved for temperature:
# temperature = initial_heat_energy / (fluid_mass * fluid_heat_cap + (1 - porosity) * rock_density * rock_heat_cap * (1 - 0.01*t))
#
# Parameters:
# Bulk modulus = 2
# Shear modulus = 1.5
# fluid bulk modulus = 0.5
# initial porepressure = 0.1
# initial temperature = 10
#
# Desired output:
# zdisp = -0.01*t
# p0 = 0.1 - 0.5*log(1-0.01*t)
# stress_xx = stress_yy = -0.01*t
# stress_zz = -0.04*t
# t0 =  11.5 / (0.159 + 0.99 * (1 - 0.01*t))
#
# Regarding the "log" - it comes from preserving fluid mass
#
# Note that the PorousFlowMassVolumetricExpansion and PorousFlowHeatVolumetricExpansion Kernels are used
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
  [pp]
    initial_condition = 0.1
  []
  [temp]
    initial_condition = 10
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
  [top_velocity]
    type = FunctionDirichletBC
    variable = disp_z
    function = -0.01*t
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
    variable = pp
    fluid_component = 0
  []
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [temp]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
  [poro_vol_exp_temp]
    type = PorousFlowHeatVolumetricExpansion
    variable = temp
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

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 1
    viscosity = 1
    thermal_expansion = 0
    cv = 1.3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
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
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.5 0 0   0 0.5 0   0 0 0.5'
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '0 0 0'
    variable = pp
  []
  [t0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '0 0 0'
    variable = temp
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
  [total_heat]
    type = PorousFlowHeatEnergy
    phase = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
  [rock_heat]
    type = PorousFlowHeatEnergy
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
  [fluid_heat]
    type = PorousFlowHeatEnergy
    include_porous_skeleton = false
    phase = 0
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
  dt = 2
  end_time = 10
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = heat03
  [csv]
    type = CSV
  []
[]
