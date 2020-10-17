[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
[]

[Adaptivity]
  marker = 'box'
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '1 1 0 '
      inside = 'refine'
      outside = 'do_nothing'
    []
  []
[]

[Variables]
  [diffused]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = 'diffused'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = 'diffused'
    boundary = 'left'
    value = 1.0
  []
  [right]
    type = DirichletBC
    variable = 'diffused'
    boundary = 'right'
    value = 0.0
  []
[]

[Constraints]
  [y_top]
    type = EqualValueBoundaryConstraint
    variable = 'diffused'
    primary = '45'
    secondary = 'top'
    penalty = 10e6
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  num_steps = 3
  nl_rel_tol = 1e-06
  nl_abs_tol = 1e-08
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
