[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
    initial_condition = 0.1
  []
  [v]
    initial_condition = 0.1
  []
[]

[Materials]
  [Du]
    type = ADCoupledValueFunctionMaterial
    function = x
    v = v
    prop_name = Du
  []
  [Dv]
    type = ADCoupledValueFunctionMaterial
    function = x^2
    v = u
    prop_name = Dv
  []
[]

[Kernels]
  [diff_u]
    type = ADMatDiffusion
    diffusivity = Du
    variable = u
  []
  [dudt]
    type = ADTimeDerivative
    variable = u
  []
  [diff_v]
    type = ADMatDiffusion
    diffusivity = Dv
    variable = v
  []
  [dvdt]
    type = ADTimeDerivative
    variable = v
  []
[]

[BCs]
  [u_left]
    type = DirichletBC
    boundary = left
    variable = u
    value = 1
  []
  [u_right]
    type = DirichletBC
    boundary = right
    variable = u
    value = 0.1
  []
  [v_top]
    type = DirichletBC
    boundary = top
    variable = v
    value = 1
  []
  [v_bottom]
    type = DirichletBC
    boundary = bottom
    variable = v
    value = 0.1
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
