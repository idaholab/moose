[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    components = 2
    fv = true
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVArrayDiffusion
    variable = u
    coeff = dc
  []
  [reaction]
    type = FVArrayReaction
    variable = u
    coeff = rc
  []
  [diffv]
    type = FVDiffusion
    variable = v
    coeff = d1
  []
  [vu]
    type = FVArrayCoupledForce
    variable = u
    v = v
    coef = '0 0.5'
  []
  [uv]
    type = FVCoupledArrayForce
    variable = v
    v = u
    coef = '0.05 0'
  []
[]

[FVBCs]
  [left]
    type = FVArrayDirichletBC
    variable = u
    boundary = 1
    value = '0 0'
  []

  [right]
    type = FVArrayDirichletBC
    variable = u
    boundary = 2
    value = '1 2'
  []

  [leftv]
    type = FVDirichletBC
    variable = v
    boundary = 1
    value = 0
  []

  [rightv]
    type = FVDirichletBC
    variable = v
    boundary = 2
    value = 2
  []
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    prop_names = 'd1'
    prop_values = 1
  []
  [dc]
    type = ADGenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [rc]
    type = ADGenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
