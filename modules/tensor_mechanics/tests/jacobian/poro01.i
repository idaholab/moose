# tests of the poroelasticity kernel, PoroMechanicsCoupling
# in conjunction with the usual StressDivergenceTensors Kernel
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./p]
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    variable = disp_x
    min = -1
    max = 1
  [../]
  [./disp_y]
    type = RandomIC
    variable = disp_y
    min = -1
    max = 1
  [../]
  [./disp_z]
    type = RandomIC
    variable = disp_z
    min = -1
    max = 1
  [../]
  [./p]
    type = RandomIC
    variable = p
    min = -1
    max = 1
  [../]
[]

[Kernels]
  [./unimportant_p]
    type = TimeDerivative
    variable = p
  [../]
  [./grad_stress_x]
    type = StressDivergenceTensors
    displacements = 'disp_x disp_y disp_z'
    variable = disp_x
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
  [./poro_x]
    type = PoroMechanicsCoupling
    variable = disp_x
    porepressure = p
    component = 0
  [../]
  [./poro_y]
    type = PoroMechanicsCoupling
    variable = disp_y
    porepressure = p
    component = 1
  [../]
  [./poro_z]
    type = PoroMechanicsCoupling
    variable = disp_z
    porepressure = p
    component = 2
  [../]
  [./This_is_not_poroelasticity._It_is_checking_diagonal_jacobian]
    type = PoroMechanicsCoupling
    variable = disp_x
    porepressure = disp_x
    component = 0
  [../]
  [./This_is_not_poroelasticity._It_is_checking_diagonal_jacobian_again]
    type = PoroMechanicsCoupling
    variable = disp_x
    porepressure = disp_x
    component = 1
  [../]
  [./This_is_not_poroelasticity._It_is_checking_offdiagonal_jacobian_for_disps]
    type = PoroMechanicsCoupling
    variable = disp_x
    porepressure = disp_y
    component = 2
  [../]
[]


[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./biot]
    type = GenericConstantMaterial
    prop_names = biot_coefficient
    prop_values = 0.54
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
[]
