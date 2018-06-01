[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
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
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Postprocessors]
  [self]
    type = PerfGraphTime
    section_name = FEProblem::computeResidualInternal
    time_type = SELF
  []
  [children]
    type = PerfGraphTime
    section_name = FEProblem::computeResidualInternal
    time_type = CHILDREN
  []
  [total]
    type = PerfGraphTime
    section_name = FEProblem::computeResidualInternal
    time_type = TOTAL
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  [pgraph]
    type = PerfGraphOutput
    level = 3
    heaviest_branch = true
    heaviest_sections = 10
  []
[]
