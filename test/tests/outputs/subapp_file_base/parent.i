[Mesh]
  type = GeneratedMesh
  dim = 1
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

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    execute_on = initial
    positions = '0 0 0'
    input_files = sub_shortcut.i
  []
[]

[Outputs]
  console = false
[]
