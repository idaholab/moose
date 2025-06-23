[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u][]
  [v][]
[]

[GPUKernels]
  [diff]
    type = GPUDiffusion
    variable = u
  []
  [diff_v]
    type = GPUDiffusion
    variable = v
  []
[]

[GPUBCs]
  [left]
    type = GPUDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = GPUCoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = v
  []
  [v_left]
    type = GPUDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = GPUDirichletBC
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
