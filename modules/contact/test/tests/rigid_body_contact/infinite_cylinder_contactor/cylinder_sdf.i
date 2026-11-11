# Verify InfiniteCylinderContactor's signed distance and outward normal
# on a 5^3 hex grid spanning [-2, 2]^3.
#
# Two reference contactors are sampled on the same grid:
#
#   * `cyl_x`: axis-aligned cylinder centered on the origin with axis =
#     x_hat and radius 1.  Analytic SDF at a point (x, y, z) is
#     sqrt(y^2 + z^2) - 1, so it is easy to hand-check against the
#     `sdf_analytic` FunctionAux column below.
#
#   * `cyl_tilted`: same radius, but the axis is the (1, 1, 0)/sqrt(2)
#     direction through the origin.  Exercises the projector part of
#     the SDF and the normalization applied to the user-supplied
#     `axis` (which is deliberately not a unit vector here).
#
# The gold CSV pins every sampled column; any regression that shifts
# the SDF sign or the projector by even a small numerical amount fails
# the CSVDiff.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
    xmin = -2
    xmax = 2
    ymin = -2
    ymax = 2
    zmin = -2
    zmax = 2
  []
[]

[UserObjects]
  [cyl_x]
    type = InfiniteCylinderContactor
    origin = '0 0 0'
    axis   = '1 0 0'
    radius = 1.0
  []
  [cyl_tilted]
    type = InfiniteCylinderContactor
    origin = '0 0 0'
    # Not a unit vector on purpose -- contactor normalizes internally.
    axis   = '2 2 0'
    radius = 1.0
  []
[]

[AuxVariables]
  [sdf_cyl_x]
    family = LAGRANGE
    order  = FIRST
  []
  [nx_cyl_x]
    family = LAGRANGE
    order  = FIRST
  []
  [ny_cyl_x]
    family = LAGRANGE
    order  = FIRST
  []
  [nz_cyl_x]
    family = LAGRANGE
    order  = FIRST
  []
  [sdf_cyl_tilted]
    family = LAGRANGE
    order  = FIRST
  []
  [nx_cyl_tilted]
    family = LAGRANGE
    order  = FIRST
  []
  [ny_cyl_tilted]
    family = LAGRANGE
    order  = FIRST
  []
  [nz_cyl_tilted]
    family = LAGRANGE
    order  = FIRST
  []
  # Analytic reference SDF for the x-axis cylinder.
  [sdf_analytic]
    family = LAGRANGE
    order  = FIRST
  []
[]

[AuxKernels]
  [sdf_cyl_x]
    type = LevelSetContactorAux
    variable  = sdf_cyl_x
    contactor = cyl_x
    quantity  = signed_distance
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [nx_cyl_x]
    type = LevelSetContactorAux
    variable  = nx_cyl_x
    contactor = cyl_x
    quantity  = normal_x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ny_cyl_x]
    type = LevelSetContactorAux
    variable  = ny_cyl_x
    contactor = cyl_x
    quantity  = normal_y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [nz_cyl_x]
    type = LevelSetContactorAux
    variable  = nz_cyl_x
    contactor = cyl_x
    quantity  = normal_z
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [sdf_cyl_tilted]
    type = LevelSetContactorAux
    variable  = sdf_cyl_tilted
    contactor = cyl_tilted
    quantity  = signed_distance
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [nx_cyl_tilted]
    type = LevelSetContactorAux
    variable  = nx_cyl_tilted
    contactor = cyl_tilted
    quantity  = normal_x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ny_cyl_tilted]
    type = LevelSetContactorAux
    variable  = ny_cyl_tilted
    contactor = cyl_tilted
    quantity  = normal_y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [nz_cyl_tilted]
    type = LevelSetContactorAux
    variable  = nz_cyl_tilted
    contactor = cyl_tilted
    quantity  = normal_z
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [sdf_analytic]
    type = FunctionAux
    variable = sdf_analytic
    function = sdf_analytic_func
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  # Analytic SDF for a cylinder along +x through the origin, radius 1.
  [sdf_analytic_func]
    type = ParsedFunction
    expression = 'sqrt(y*y + z*z) - 1'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  # A handful of point samples: hand-checkable values that pin the
  # contactor's numerics into the gold.
  [sdf_at_2_1_0]
    type = PointValue
    variable = sdf_cyl_x
    point = '2 1 0'
  []
  [sdf_at_0_0_2]
    type = PointValue
    variable = sdf_cyl_x
    point = '0 0 2'
  []
  [nx_at_1_2_0]
    type = PointValue
    variable = nx_cyl_x
    point = '1 2 0'
  []
  [ny_at_1_2_0]
    type = PointValue
    variable = ny_cyl_x
    point = '1 2 0'
  []
  [nz_at_1_2_0]
    type = PointValue
    variable = nz_cyl_x
    point = '1 2 0'
  []
  [sdf_tilted_at_1_-1_0]
    type = PointValue
    variable = sdf_cyl_tilted
    point = '1 -1 0'
  []
  # Error norms of the SDF field for the axis-aligned cylinder vs the
  # analytic function.  Any implementation regression that biases the
  # SDF here will move the L2 norm.
  [sdf_l2_err]
    type = ElementL2Difference
    variable = sdf_cyl_x
    other_variable = sdf_analytic
  []
[]

[Outputs]
  csv = true
[]
