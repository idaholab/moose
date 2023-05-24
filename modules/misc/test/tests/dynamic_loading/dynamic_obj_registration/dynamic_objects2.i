# This input file contains some objects only available through heat_conduction
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 2
  xmax = 50
  ymax = 25
  elem_type = QUAD4
  uniform_refine = 2
[]

[Variables]
  [c]
    order = THIRD
    family = HERMITE
  []
[]

[ICs]
  [c_IC]
    type = BoundingBoxIC
    x1 = 15.0
    x2 = 35.0
    y1 = 0.0
    y2 = 25.0
    inside = 1.0
    outside = -0.8
    variable = c
  []
[]

[Kernels]
  [ie_c]
    type = TimeDerivative
    variable = c
  []
  [d]
    type = Diffusion
    variable = c
  []
  [s]
    type = HeatSource
    variable = c
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
    []
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 15
  nl_max_its = 10
  start_time = 0.0

  num_steps = 2
  dt = 1.0
[]

[Problem]
  register_objects_from = 'HeatConductionApp'
  library_path = '../../../../../heat_conduction/lib'
[]
