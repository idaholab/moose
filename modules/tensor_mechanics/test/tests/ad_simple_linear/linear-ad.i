[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[ADKernels]
  [./stress_x]
    type = ADStressDivergenceTest
    component = 0
    variable = disp_x
  [../]
  [./stress_y]
    type = ADStressDivergenceTest
    component = 1
    variable = disp_y
  [../]
  [./stress_z]
    type = ADStressDivergenceTest
    component = 2
    variable = disp_z
  [../]
[]

[BCs]
  [./symmy]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = PresetBC
    variable = disp_z
    boundary = front
    value = 0.1
  [../]
[]

[ADMaterials]
  [./linear]
    type = TensorMechADMatTest
    displacements = 'disp_x disp_y disp_z'
    poissons_ratio = 0.3
    youngs_modulus = 1e10
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.05

  #Preconditioned JFNK (default)
  solve_type = 'NEWTON'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomeramg

  dtmin = 0.05
  num_steps = 1
[]

[Outputs]
  exodus = true
  file_base = "linear-out"
[]
