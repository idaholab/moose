[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [ue]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [ve]
  []

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

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = ue
    diffusion_coefficient = dce
  []
  [reaction]
    type = ArrayReaction
    variable = ue
    reaction_coefficient = rce
  []
  [diffv]
    type = Diffusion
    variable = ve
  []
  [vu]
    type = ArrayCoupledForce
    variable = ue
    v = ve
    coef = '0 0.5'
  []
  [uv]
    type = CoupledArrayForce
    variable = ve
    v = ue
    coef = '0.05 0'
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = ue
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ArrayDirichletBC
    variable = ue
    boundary = 2
    values = '1 2'
  []

  [leftv]
    type = DirichletBC
    variable = ve
    boundary = 1
    value = 0
  []

  [rightv]
    type = DirichletBC
    variable = ve
    boundary = 2
    value = 2
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
  [dce]
    type = GenericConstantArray
    prop_name = dce
    prop_value = '1 1'
  []
  [rce]
    type = GenericConstant2DArray
    prop_name = rce
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
