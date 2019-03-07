# Test that porosity is correctly calculated.
# Porosity = biot + (phi0 - biot) * exp(-vol_strain + (biot - 1) / solid_bulk * (porepressure - ref_pressure))
# The parameters used are:
# biot = 0.7
# phi0 = 0.5
# vol_strain = 0.5
# solid_bulk = 0.3
# porepressure = 2
# ref_pressure = 3
# which yield porosity = 0.370255745860
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  PorousFlowDictator = dictator
  displacements = 'disp_x disp_y disp_z'
  biot_coefficient = 0.7
[]

[Variables]
  [./porepressure]
    initial_condition = 2
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[ICs]
  [./disp_x]
    type = FunctionIC
    function = '0.5 * x'
    variable = disp_x
  [../]
[]

[Kernels]
  [./dummy_p]
    type = TimeDerivative
    variable = porepressure
  [../]
  [./dummy_x]
    type = TimeDerivative
    variable = disp_x
  [../]
  [./dummy_y]
    type = TimeDerivative
    variable = disp_y
  [../]
  [./dummy_z]
    type = TimeDerivative
    variable = disp_z
  [../]
[]

[AuxVariables]
  [./porosity]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./porosity]
    type = PorousFlowPropertyAux
    property = porosity
    variable = porosity
  [../]
[]

[Postprocessors]
  [./porosity]
    type = PointValue
    variable = porosity
    point = '0 0 0'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
  [./pc]
    type = PorousFlowCapillaryPressureConst
  [../]
[]

[Modules]
  [./FluidProperties]
    [./simple_fluid]
      type = SimpleFluidProperties
    [../]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = 3
  [../]
  [./eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
    at_nodes = false
  [../]
  [./vol_strain]
    type = PorousFlowVolumetricStrain
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    at_nodes = false
    porepressure = porepressure
    capillary_pressure = pc
  [../]
  [./porosity]
    type = PorousFlowPorosity
    at_nodes = false
    fluid = true
    mechanical = true
    ensure_positive = false
    porosity_zero = 0.5
    solid_bulk = 0.3
    reference_porepressure = 3
  [../]
[]

[Executioner]
  solve_type = Newton
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
