# Tests the PorousFlowMassVolumetricExpansion kernel
# Fluid with constant bulk modulus, van-Genuchten capillary, HM porosity
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
  displacements = 'disp_x disp_y disp_z'
  block = 0
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./porepressure]
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  [../]
  [./disp_y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  [../]
  [./disp_z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  [../]
  [./p]
    type = RandomIC
    min = -1
    max = 1
    variable = porepressure
  [../]
[]

[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    component = 2
  [../]
  [./poro]
    type = PorousFlowMassVolumetricExpansion
    component_index = 0
    displacements = 'disp_x disp_y disp_z'
    variable = porepressure
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
  [./simple1]
    type = TensorMechanicsPlasticSimpleTester
    a = 0
    b = 1
    strength = 1E20
    yield_function_tolerance = 1.0E-9
    internal_constraint_tolerance = 1.0E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress #MultiPlasticityStress
    #ep_plastic_tolerance = 1E-9
    #plastic_models = 'simple1'
  [../]

  [./ppss]
    type = PorousFlowMaterial1PhaseP_VG
    porepressure = porepressure
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./porosity]
    type = PorousFlowPorosityHM
    porosity_zero = 0.1
    biot_coefficient = 0.5
    solid_bulk = 1
  [../]
  [./p_eff]
    type = PorousFlowMaterialEffectiveFluidPressure
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-5
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jacobian2
  exodus = false
[]
