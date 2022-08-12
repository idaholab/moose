[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    ix = '2 2'
    dy = '1 1'
    iy = '2 2'
    subdomain_id = '0 0 0 1'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[DGKernels]
  [dgdiff]
    type = ArrayDGDiffusion
    variable = u
    diff = dc
  []
[]

[BCs]
  [left]
    type = ArrayVacuumBC
    variable = u
    boundary = 1
  []

  [right]
    type = ArrayPenaltyDirichletBC
    variable = u
    boundary = 2
    value = '1 2'
    penalty = 4
  []
[]

[Materials]
  [dc0]
    type = GenericConstantArray
    block = 0
    prop_name = dc
    prop_value = '1 1'
  []
  [dc1]
    type = GenericConstantArray
    block = 1
    prop_name = dc
    prop_value = '2 1'
  []
  [rc]
    type = GenericConstant2DArray
    block = '0 1'
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[AuxVariables]
  [u_tag_x]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
  [u_tag_y]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[AuxKernels]
  [u_tag_x]
    type = TagVectorArrayVariableGradientAux
    variable = u_tag_x
    v = u
    grad_component = x
    vector_tag = 'SOLUTION'
  []
  [u_tag_y]
    type = TagVectorArrayVariableGradientAux
    variable = u_tag_y
    v = u
    grad_component = y
    vector_tag = 'NONTIME'
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
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
