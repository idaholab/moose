[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
  uniform_refine = 4
[]

[TestGetActions]
[]

[Variables]
  [./convected]
  [../]
  [./diffused]
  [../]
[]

[Kernels]
  # intentionally give a name the same as material names
  [./mat1]
    type = Diffusion
    variable = convected
  [../]
  [./diff_u]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'left_convected right_convected left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 0
  [../]

  [./right_convected]
    type = DirichletBC
    variable = convected
    boundary = '2'
    value = 1
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 1
  [../]
[]

[Materials]
  [./mat4]
   type = RandomMaterial
   block = 0
  [../]
  [./mat3]
   type = MTMaterial
   block = 0
  [../]
  [./mat1]
   type = GenericConstantMaterial
   prop_names = prop1
   prop_values = 1.0
   block = 0
  [../]
  [./mat2]
   type = CoupledMaterial
   mat_prop = prop2
   coupled_mat_prop = prop1
   block = 0
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
[]
