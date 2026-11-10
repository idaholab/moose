[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
[]

[Postprocessors]
  [avg_u]
    type = AverageNodalVariableValue
    variable = u
  []
[]

[Outputs]
  file_base = common_file_base_names
  [exodus_a]
    type = Exodus
  []
  [exodus_b]
    type = Exodus
  []
  [csv_a]
    type = CSV
  []
[]
