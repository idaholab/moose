[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [v]
    components = 2
  []
[]

[AuxVariables]
  [u]
    components = 2
  []
[]

[Kernels]
  [diff_v]
    type = ArrayDiffusion
    variable = v
    diffusion_coefficient = dc
  []
  [force_v]
    type = ArrayCoupledForce
    variable = v
    v = u
    is_v_array = true
    coef = '1 1'
  []
  [time_v]
    type = ArrayTimeDerivative
    variable = v
    time_derivative_coefficient = tc
  []
[]

[BCs]
  [left_v]
    type = ArrayDirichletBC
    variable = v
    boundary = left
    values = '2 2'
  []
  [right_v]
    type = ArrayDirichletBC
    variable = v
    boundary = right
    values = '1 1'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [tc]
    type = GenericConstantArray
    prop_name = tc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

