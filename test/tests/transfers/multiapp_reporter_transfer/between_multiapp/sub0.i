[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[VectorPostprocessors]
  [base_sub0_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '30 30 30; 40 40 40'
  []
  [from_sub1_vpp]
    type = ConstantVectorPostprocessor
    vector_names = 'a b'
    value = '10 10 10 ; 20 20 20'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = none
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
