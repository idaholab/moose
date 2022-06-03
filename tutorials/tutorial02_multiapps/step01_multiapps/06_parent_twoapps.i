[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 40
  nz = 40
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
  [force]
    type = BodyForce
    variable = u
    value = 1.
  []
  [td]
    type = TimeDerivative
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
    value = 0
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  dt = 1.

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]

[MultiApps]
  [app1]
    type = TransientMultiApp
    positions   = '0 0 0  1 0 0  2 0 0'
    input_files = '06_sub_twoapps.i'
  []

  [app2]
    type = TransientMultiApp
    positions   = '0 0 0  1 0 0'
    input_files = '06_sub_twoapps.i'
  []
[]
