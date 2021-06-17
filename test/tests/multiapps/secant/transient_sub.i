[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [u]
  []
[]

[Kernels]
  [time]
    type = CoefTimeDerivative
    variable = v
    Coefficient = 0.1
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [force_v]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Postprocessors]
  [vnorm]
    type = ElementL2Norm
    variable = v
  []
[]

[Executioner]
  type = Transient
  end_time = 10
  nl_abs_tol = 1e-12
  steady_state_detection = true
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  fixed_point_algorithm = 'secant'
[]

[Outputs]
  [csv]
    type = CSV
    start_step = 6
  []
  exodus = false
[]
