# FullySaturatedMassTimeDerivative
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  displacements = 'disp_x disp_y disp_z'
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.5
    bulk_modulus = 1.5
    density0 = 1.0
  []
[]

[Variables]
  [pp]
  []
  [T]
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  []
  [disp_y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  []
  [disp_z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  []
  [pp]
    type = RandomIC
    variable = pp
    min = 0
    max = 1
  []
  [T]
    type = RandomIC
    variable = T
    min = 0
    max = 1
  []
[]

[BCs]
  # necessary otherwise volumetric strain rate will be zero
  [disp_x]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  []
  [disp_y]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'left right'
  []
  [disp_z]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'left right'
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowFullySaturatedMassTimeDerivative
    variable = pp
    coupling_type = ThermoHydroMechanical
    biot_coefficient = 0.9
  []
  [dummyT]
    type = TimeDerivative
    variable = T
  []
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
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp disp_x disp_y disp_z T'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [simple1]
    type = TensorMechanicsPlasticSimpleTester
    a = 0
    b = 1
    strength = 1E20
    yield_function_tolerance = 1.0E-9
    internal_constraint_tolerance = 1.0E-9
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 2.0
    shear_modulus = 3.0
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
  [temperature]
    type = PorousFlowTemperature
    temperature = T
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = the_simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst  # only the initial vaue of this is ever used
    porosity = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    biot_coefficient = 0.9
    fluid_bulk_modulus = 1.5
    solid_bulk_compliance = 0.5
  []
  [thermal_expansion]
    type = PorousFlowConstantThermalExpansionCoefficient
    biot_coefficient = 0.9
    fluid_coefficient = 0.5
    drained_coefficient = 0.4
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
  end_time = 2
[]

[Outputs]
  exodus = false
[]
