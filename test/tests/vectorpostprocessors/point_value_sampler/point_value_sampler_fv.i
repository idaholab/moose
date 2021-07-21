[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]
  [./diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1
  [../]
[]

[FVBCs]
  [./left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left_v]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[VectorPostprocessors]
  [./point_sample]
    type = PointValueSampler
    warn_discontinuous_face_values = false
    variable = 'u v'
    points = '0.09 0.09 0  0.23 0.4 0  0.78 0.2 0'
    sort_by = x
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
