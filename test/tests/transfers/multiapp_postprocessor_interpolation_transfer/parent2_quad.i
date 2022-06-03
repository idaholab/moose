[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./pp_aux]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./quad]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.1 0.1 0 0.9 0.1 0 0.1 0.9 0 0.9 0.9 0'
    input_files = 'quad_sub1.i'
  [../]
[]

[Transfers]
  [./sub_to_parent_pp]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = quad
    variable = pp_aux
    postprocessor = pp
  [../]
[]
