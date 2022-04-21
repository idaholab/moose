[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[AuxVariables]
  [u0][]
  [u1][]
[]

[AuxKernels]
  [u0]
    type = TagVectorArrayAux
    variable = u0
    v = u
    component = 0
    execute_on = 'timestep_end'
    vector_tag = 'nontime'
  []
  [u1]
    type = TagVectorArrayAux
    variable = u1
    v = u
    component = 1
    execute_on = 'timestep_end'
    vector_tag = 'nontime'
  []
[]

[Variables]
  [u]
    components = 2
  []
[]

[Kernels]
  [time]
    type = ArrayTimeDerivative
    variable = u
    time_derivative_coefficient = tc
  []
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ArrayDirichletBC
    variable = u
    boundary = 2
    values = '1 2'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [tc]
    type = GenericConstantArray
    prop_name = tc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
