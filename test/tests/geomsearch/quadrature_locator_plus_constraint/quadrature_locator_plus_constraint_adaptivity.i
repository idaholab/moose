[Mesh]
  [lower]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    ymax = 0.5
    boundary_name_prefix = lower
  []
  [upper]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    ymin = 0.5
    ymax = 1.0
    boundary_name_prefix = upper
    boundary_id_offset = 10
  []
  [combine]
    type = CombinerGenerator
    inputs = 'lower upper'
  []
[]

[Problem]
  use_hash_table_matrix_assembly = true
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = T
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = T
    boundary = lower_bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = T
    boundary = upper_top
    value = 1
  []
[]

[Constraints]
  [tied]
    type = TiedValueConstraint
    variable = T
    primary_variable = T
    secondary = upper_bottom
    primary = lower_top
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_abs_tol = 1e-12
  num_steps = 3
[]

[Adaptivity]
  marker = uniform
  [Markers]
    [uniform]
      type = UniformMarker
      mark = REFINE
    []
  []
[]

[Postprocessors]
  [upper_bottom_avg]
    type = SideAverageValue
    boundary = upper_bottom
    variable = T
  []
  [lower_top_avg]
    type = SideAverageValue
    boundary = lower_top
    variable = T
  []
  [num_dofs]
    type = NumDOFs
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]
