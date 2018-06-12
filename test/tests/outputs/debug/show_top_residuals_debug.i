[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  block_id = '0 1'
  block_name = 'block_zero block_one'
[]

[MeshModifiers]
  [./subdomain_id]
    type = AssignSubdomainID
    subdomain_id = 1
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

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_top_residuals = 1
[]
