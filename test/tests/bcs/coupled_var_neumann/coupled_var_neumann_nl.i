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
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
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
    type = CoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = v
  []
  [v_left]
    type = DirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = DirichletBC
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
[]
