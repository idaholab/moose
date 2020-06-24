[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [v]
    family = LAGRANGE_VEC
  []
[]

[Kernels]
  [diff_v]
    type = ADVectorDiffusion
    variable = v
  []
[]

[BCs]
  [left_v]
    type = ADVectorFunctionDirichletBC
    variable = v
    function_x = 1
    function_y = 2
    boundary = 'left'
  []
  [right_v]
    type = ADVectorFunctionDirichletBC
    variable = v
    function_x = 4
    function_y = 8
    boundary = 'right'
  []
[]

[Materials]
  [coupled]
    type = VectorCoupledValuesMaterial
    variable = v
    request_dotdot = false
  []
[]

[AuxVariables]
  [reg_vec]
    family = MONOMIAL_VEC
  []
  [ad_vec]
    family = MONOMIAL_VEC
  []
[]

[AuxKernels]
  [reg_vec]
    type = VectorMaterialRealVectorValueAux
    property = v_value
    variable = reg_vec
  []
  [ad_vec]
    type = ADVectorMaterialRealVectorValueAux
    property = v_ad_value
    variable = ad_vec
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
