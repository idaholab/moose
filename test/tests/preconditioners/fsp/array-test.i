[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [v]
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
  [diffv]
    type = Diffusion
    variable = v
  []
  [vu]
    type = ArrayCoupledForce
    variable = u
    v = v
    coef = '0 0.5'
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

  [leftv]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  []

  [rightv]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 2
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [rc]
    type = GenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'uv'
    [uv]
      splitting = 'u v'
      # Generally speaking, there are four types of splitting we could choose
      # <additive,multiplicative,symmetric_multiplicative,schur>
      splitting_type  = symmetric_multiplicative
    []
    [u]
      vars = 'u'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
    []
    [v]
      vars = 'v'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre  preonly'
    []
  []
[]

[Postprocessors]
  [intu0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [intu1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
  [intv]
    type = ElementIntegralVariablePostprocessor
    variable = v
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
