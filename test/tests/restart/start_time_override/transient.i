[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
  [u_norm]
    type = ElementL2Norm
    variable = u
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 5
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  checkpoint = true
  csv = true
[]
