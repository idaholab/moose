[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [v_x]
  []
  [v_y]
  []
[]

[ICs]
  [v_x]
    type = FunctionIC
    variable = v_x
    function = sin(2*y*pi)
  []
  [v_y]
    type = FunctionIC
    variable = v_y
    function = cos(2*x*pi)
  []
[]

[Kernels]
  [diff_x]
    type = Diffusion
    variable = v_x
  []
  [dt_x]
    type = TimeDerivative
    variable = v_x
  []
  [diff_y]
    type = Diffusion
    variable = v_y
  []
  [dt_y]
    type = TimeDerivative
    variable = v_y
  []
[]

[UserObjects]
  [renormalize]
    type = PointwiseRenormalizeVector
    v = 'v_x v_y'
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 0.01
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
