[Problem]
  nl_sys_names = 'nl0 nl1'
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Variables]
  [u]
    solver_sys = nl0
  []
  [v]
    solver_sys = nl1
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [coupled_uv]
    type = ADMatCoupledForce
    variable = u
    v = v
    mat_prop_coef = u_coef
  []
  [coupled_vu]
    type = ADCoupledForce
    variable = v
    v = u
    coef = 2
  []
[]

[Materials]
  [u_coef]
    type = ADParsedMaterial
    property_name = u_coef
    coupled_variables = 'u v'
    expression = '1 + u * u + v'
  []
[]

[BCs]
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_v]
    type = ADDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  console = false
[]
