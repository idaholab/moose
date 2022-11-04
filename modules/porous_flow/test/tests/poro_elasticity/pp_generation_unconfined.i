# A sample is constrained on all sides, except its top
# and its boundaries are
# also impermeable.  Fluid is pumped into the sample via a
# volumetric source (ie kg/second per cubic meter), and the
# rise in the top surface, porepressure, and stress are observed.
#
# In the standard poromechanics scenario, the Biot Modulus is held
# fixed and the source has units 1/time.  Then the expected result
# is
# strain_zz = disp_z = BiotCoefficient*BiotModulus*s*t/((bulk + 4*shear/3) + BiotCoefficient^2*BiotModulus)
# porepressure = BiotModulus*(s*t - BiotCoefficient*strain_zz)
# stress_xx = (bulk - 2*shear/3)*strain_zz   (remember this is effective stress)
# stress_zz = (bulk + 4*shear/3)*strain_zz   (remember this is effective stress)
#
# In porous_flow, however, the source has units kg/s/m^3 and the
# Biot Modulus is not held fixed.  This means that disp_z, porepressure,
# etc are not linear functions of t.  Nevertheless, the ratios remain
# fixed:
# stress_xx/strain_zz = (bulk - 2*shear/3) = 1 (for the parameters used here)
# stress_zz/strain_zz = (bulk + 4*shear/3) = 4 (for the parameters used here)
# porepressure/strain_zz = 13.3333333 (for the parameters used here)

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

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
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
  [confinez]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'back'
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
  [flux]
    type = PorousFlowAdvectiveFlux
    variable = porepressure
    gravity = '0 0 0'
    fluid_component = 0
  []
  [source]
    type = BodyForce
    function = 0.1
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
    bulk_modulus = 3.3333333333
    density0 = 1
    thermal_expansion = 0
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
    displacements = 'disp_x disp_y disp_z'
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
    type = PorousFlowPorosity
    fluid = true
    mechanical = true
    porosity_zero = 0.1
    biot_coefficient = 0.3
    solid_bulk = 2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0   0 1 0   0 0 1' # unimportant
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0 # unimportant in this fully-saturated situation
    phase = 0
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = none
    point = '0 0 0'
    variable = porepressure
  []
  [zdisp]
    type = PointValue
    outputs = none
    point = '0 0 0.5'
    variable = disp_z
  []
  [stress_xx]
    type = PointValue
    outputs = none
    point = '0 0 0'
    variable = stress_xx
  []
  [stress_yy]
    type = PointValue
    outputs = none
    point = '0 0 0'
    variable = stress_yy
  []
  [stress_zz]
    type = PointValue
    outputs = none
    point = '0 0 0'
    variable = stress_zz
  []
  [stress_xx_over_strain]
    type = FunctionValuePostprocessor
    function = stress_xx_over_strain_fcn
    outputs = csv
  []
  [stress_zz_over_strain]
    type = FunctionValuePostprocessor
    function = stress_zz_over_strain_fcn
    outputs = csv
  []
  [p_over_strain]
    type = FunctionValuePostprocessor
    function = p_over_strain_fcn
    outputs = csv
  []
[]

[Functions]
  [stress_xx_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'stress_xx zdisp'
  []
  [stress_zz_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'stress_zz zdisp'
  []
  [p_over_strain_fcn]
    type = ParsedFunction
    expression = a/b
    symbol_names = 'a b'
    symbol_values = 'p0 zdisp'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-14 1E-10 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = 0
  end_time = 10
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = pp_generation_unconfined
  [csv]
    type = CSV
  []
[]
