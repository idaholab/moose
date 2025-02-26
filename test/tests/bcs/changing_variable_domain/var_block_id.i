[Mesh]
  add_subdomain_ids = '1 2'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 4
    ymax = 4
    nx = 2
    ny = 2
    subdomain_ids = '0 2 0 2'
  []
[]

[Problem]
  kernel_coverage_check = false
  boundary_restricted_elem_integrity_check = false
[]

[Variables]
  [u]
    block = '0 2'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  # Both defined across block 0 and 1 once moving
  [diri]
    type = DirichletBC
    variable = u
    boundary = 'top'
    value = '1'
  []
  [neum]
    type = NeumannBC
    variable = u
    boundary = 'bottom'
    value = '1'
    skip_execution_outside_variable_domain = true
  []

  # There to get a solution
  [diri_left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = '0'
  []
[]

[MeshModifiers]
  [move_0_to_1]
    type = TimedSubdomainModifier
    execute_on = 'TIMESTEP_BEGIN'
    times = '1.5'
    blocks_from = '2'
    blocks_to = '1'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 4

  nl_abs_tol = '1e-12'
[]

[Outputs]
  exodus = true
[]
