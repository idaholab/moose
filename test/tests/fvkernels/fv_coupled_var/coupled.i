[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
[]

[Variables]
  [u][]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [w]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [s][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [rxn]
    type = Reaction
    variable = u
    rate = 2.0
  []
  [diffs]
    type = Diffusion
    variable = s
  []
  [prod]
    type = CoupledForce
    variable = s
    v = u
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
  [rxn]
    type = FVReaction
    variable = v
    rate = 2.0
  []
  [diffw]
    type = FVDiffusion
    variable = w
    coeff = coeff
  []
  [prod]
    type = FVCoupledForce
    variable = w
    v = 'v'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
  [leftw]
    type = FVDirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [rightw]
    type = FVDirichletBC
    variable = w
    boundary = right
    value = 1
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
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
  [lefts]
    type = DirichletBC
    variable = s
    boundary = left
    value = 0
  []
  [rights]
    type = DirichletBC
    variable = s
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
