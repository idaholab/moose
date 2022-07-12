[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [./subdomains]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.1 0.1 0'
    block_id = 1
    top_right = '0.9 0.9 0'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./block]
    type = GenericConstantMaterial
    block = '0 1'
    prop_names = 'property0 property1 property2 property3 property4 property5 property6 property7 property8 property9 property10'
    prop_values = '0 1 2 3 4 5 6 7 8 9 10'
  [../]
  [./boundary]
    type = GenericConstantMaterial
    prop_names = bnd_prop
    boundary = top
    prop_values = 12345
  [../]
  [./restricted]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'restricted0 restricted1'
    prop_values = '10 11'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  [./debug] # This is only a test, you should turn this on via [Debug] block
    type = MaterialPropertyDebugOutput
  [../]
[]
