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
  [w]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [x]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diffu]
    type = KokkosArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [diffv]
    type = KokkosDiffusion
    variable = v
  []
  [diffw]
    type = KokkosArrayDiffusion
    variable = w
    diffusion_coefficient = dc
  []
  [diffx]
    type = KokkosArrayDiffusion
    variable = x
    diffusion_coefficient = dc
  []
  [vu]
    type = KokkosArrayCoupledForce
    variable = u
    v = v
    coef = '0 0.5'
  []
  [wu]
    type = KokkosArrayCoupledForce
    variable = u
    v = w
    is_v_array = true
    coef = '1 -0.5'
  []
  [wxu]
    type = KokkosArrayCoupledArraysForce
    variable = u
    v = 'w x'
    coef = 0.25
  []
[]

[BCs]
  [leftu]
    type = KokkosArrayDirichletBC
    variable = u
    boundary = left
    values = '0 0'
  []
  [rightu]
    type = KokkosArrayDirichletBC
    variable = u
    boundary = right
    values = '1 2'
  []
  [leftv]
    type = KokkosDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [rightv]
    type = KokkosDirichletBC
    variable = v
    boundary = right
    value = 2
  []
  [leftw]
    type = KokkosArrayDirichletBC
    variable = w
    boundary = left
    values = '1 0'
  []
  [rightw]
    type = KokkosArrayDirichletBC
    variable = w
    boundary = right
    values = '3 4'
  []
  [leftx]
    type = KokkosArrayDirichletBC
    variable = x
    boundary = left
    values = '2 -1'
  []
  [rightx]
    type = KokkosArrayDirichletBC
    variable = x
    boundary = right
    values = '0 1'
  []
[]

[Materials]
  [dc]
    type = KokkosGenericConstantMaterial
    prop_names = dc
    prop_values = 1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
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
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
