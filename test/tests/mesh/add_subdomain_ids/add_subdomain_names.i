[Problem]
  solve = false
  kernel_coverage_check = false
  material_coverage_check = false
[]

Box2_inactive_name = 'Box2_inactive'
Box3_inactive_name = 'Box3_inactive'
Box4_inactive_name = 'Box4_inactive'
inactive_domain_block_names = '${Box2_inactive_name} ${Box3_inactive_name} ${Box4_inactive_name}'

[Mesh]
  [BaseMesh]
    type = GeneratedMeshGenerator
    elem_type = TET4
    dim = 3
    nx = 4
    ny = 3
    nz = 2
    xmin = -10
    xmax = +10
    ymin = -10
    ymax = +10
    zmin = -2
    zmax = +2
  []

  [Box1]
    type = SubdomainBoundingBoxGenerator
    input = "BaseMesh"
    block_id = 1
    location = "INSIDE"
    bottom_left = "-20 -20 -2"
    top_right = "+20 +20 +2"
  []

  [Box2]
    type = SubdomainBoundingBoxGenerator
    input = "Box1"
    block_id = 2
    location = "INSIDE"
    bottom_left = "-4 -3 +3"
    top_right = "0 +3 0"
  []

  [Box3]
    type = SubdomainBoundingBoxGenerator
    input = "Box2"
    block_id = 3
    location = "INSIDE"
    bottom_left = "0 -3 +2"
    top_right = "+4 +3 0"
  []

  add_subdomain_names = ${inactive_domain_block_names}
[]

[AuxVariables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

# move elements between subdomains back and forth
[UserObjects]
  [GlobalSubdomainModifier]
    type = TimedSubdomainModifier
    times = '      0.4            0.6              0.4'
    blocks_from = '2              Box2_inactive    3'
    blocks_to = '  Box2_inactive  2                Box3_inactive'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient

  end_time = 1.0
  [TimeSteppers]
    [BlockEventTimeStepper]
      type = TimeSequenceStepper
      time_sequence = '0.0 0.2 0.4 0.5 0.6 1.0'
    []
  []

  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'

  nl_abs_tol = 1E-3
  nl_max_its = 400

  l_tol = 1E-3
  l_max_its = 200
[]

[Outputs]
  exodus = true
[]
