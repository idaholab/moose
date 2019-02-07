[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
  [v]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[ADKernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [convection]
    type = ADCoupledVectorConvection
    variable = u
    velocity_vector = v
  []
  [diff_v]
    type = ADVectorDiffusion
    variable = v
  []
[]

[ADBCs]
  [left]
    type = ADFunctionDirichletBC
    variable = u
    function = 1
    boundary = 'left'
  []
  [right]
    type = ADFunctionDirichletBC
    variable = u
    function = 2
    boundary = 'bottom'
  []

  [left_v]
    type = ADLagrangeVecFunctionDirichletBC
    variable = v
    x_exact_soln = 1
    y_exact_soln = 2
    boundary = 'left'
  []
  [right_v]
    type = ADLagrangeVecFunctionDirichletBC
    variable = v
    x_exact_soln = 4
    y_exact_soln = 8
    boundary = 'top'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 10
  dt = 0.05
[]

[Outputs]
  execute_on = TIMESTEP_END
  exodus = true
[]
