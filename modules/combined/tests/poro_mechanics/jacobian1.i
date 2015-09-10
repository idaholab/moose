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
  porepressure = porepressure
  block = 0
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
    type = PoroFullSatTimeDerivative
    variable = porepressure
  [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./poro_material]
    type = PoroFullSatMaterial
    porosity0 = 0.1
    biot_coefficient = 0.6
    solid_bulk_compliance = 0.25
    fluid_bulk_compliance = 0.125
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
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jacobian1
  exodus = false
[]
