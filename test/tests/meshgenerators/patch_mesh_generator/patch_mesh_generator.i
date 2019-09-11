[Mesh]
  [./patch]
    type = PatchMeshGenerator
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
  type = Transient
  solve_type = NEWTON
  start_time = 0.0
  end_time = 1.0
  dt = 1.0
[]

[Outputs]
  file_base = patch_out.e
  exodus = true
[]
