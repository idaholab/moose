# tests of the poroelasticity kernel, PoroMechanicsCoupling
# in conjunction with the usual StressDivergenceTensors Kernel
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  disp_z = disp_z
  disp_y = disp_y
  disp_x = disp_x
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
    variable = disp_x
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./poro_x]
    type = PoroMechanicsCoupling
    variable = disp_x
    coefficient = 0.54
    porepressure = p
    component = 0
  [../]
  [./poro_y]
    type = PoroMechanicsCoupling
    variable = disp_y
    coefficient = 0.54
    porepressure = p
    component = 1
  [../]
  [./poro_z]
    type = PoroMechanicsCoupling
    variable = disp_z
    coefficient = 0.54
    porepressure = p
    component = 2
  [../]
  [./This_is_not_poroelasticity._It_is_checking_diagonal_jacobian]
    type = PoroMechanicsCoupling
    variable = disp_x
    coefficient = 1.23
    porepressure = disp_x
    component = 0
  [../]
  [./This_is_not_poroelasticity._It_is_checking_diagonal_jacobian_again]
    type = PoroMechanicsCoupling
    variable = disp_x
    coefficient = 1.23
    porepressure = disp_x
    component = 1
  [../]
  [./This_is_not_poroelasticity._It_is_checking_offdiagonal_jacobian_for_disps]
    type = PoroMechanicsCoupling
    variable = disp_x
    coefficient = 1.23
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
  [../]
  [./stress]
    type = ComputeLinearElasticStress
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
