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
  active = 'check_output_file exodus_out'
  [check_output_file]
    type = OutputFileCheck
    output_object = 'exodus_out'
  []

  [exodus_out]
    type = Exodus
    file_base = 'output_FINAL'
    execute_on = 'FINAL'
  []
[]

