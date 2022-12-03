[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[AuxVariables]
  [u_tag]
    components = 2
  []
[]

[AuxKernels]
  [u_tag]
    type = TagVectorArrayVariableAux
    variable = u_tag
    v = u
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
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
