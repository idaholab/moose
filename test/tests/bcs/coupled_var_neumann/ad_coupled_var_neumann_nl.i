[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = ADCoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = v
  []
  [v_left]
    type = ADDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = ADDirichletBC
    variable = v
    boundary = 'right'
    value = 1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  file_base = coupled_var_neumann_nl_out
[]
