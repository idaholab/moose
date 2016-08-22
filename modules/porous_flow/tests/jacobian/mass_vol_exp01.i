# Tests the PorousFlowMassVolumetricExpansion kernel
# Fluid with constant bulk modulus, van-Genuchten capillary, constant porosity
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
  PorousFlowDictator = dictator
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
    fluid_component = 0
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
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress #MultiPlasticityStress
    #ep_plastic_tolerance = 1E-9
    #plastic_models = 'simple1'
  [../]

  [./vol_strain]
    type = PorousFlowVolumetricStrain
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = porepressure
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
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
