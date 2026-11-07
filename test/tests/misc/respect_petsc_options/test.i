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

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -nl0_vec_type -nl0_ghost_vec_type -aux0_vec_type -nl0_mat_type'
  petsc_options_value = 'hypre    standard      standard            standard       aij'
[]

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = u
  []
  [nl0_types]
    type = TestPetscObjectTypes
    system = nl0
    vec_type = standard
    ghost_vec_type = standard
    mat_type = aij
    outputs = none
  []
  [aux0_types]
    type = TestPetscObjectTypes
    system = aux0
    vec_type = standard
    ghost_vec_type = standard
    outputs = none
  []
[]

[Outputs]
  csv = true
[]
