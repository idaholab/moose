[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
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

[Preconditioning]
  [pre]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  [exodus_out]
    type = Exodus
  []
[]

[UserObjects]
  [check_output_file]
    type = CheckOutputFile
    output_object = 'exodus_out'
  []
[]
