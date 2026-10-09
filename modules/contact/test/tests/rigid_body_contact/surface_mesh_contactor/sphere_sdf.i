# Verify SurfaceMeshContactor's signed distance and normal against the analytic
# SphereContactor.  The surface mesh is a 2x-subdivided icosphere (80 tris,
# radius 1, origin-centered) in unit_sphere.stl.  We sample both contactors on
# the nodes of a 5^3 hex mesh spanning [-2, 2]^3 and CSV-diff the sampled
# fields.  For an 80-tri icosphere the SDF error vs analytic is roughly
# h_facet/8 ~ 5e-2, and per-component normal error is O(1e-2) except at
# points that project near a triangle edge.

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
  [sphere_analytic]
    type = SphereContactor
    center = '0 0 0'
    radius = 1.0
  []
  [sphere_mesh]
    type = SurfaceMeshContactor
    file = unit_sphere.stl
  []
[]

[AuxVariables]
  [sdf_analytic]
    family = LAGRANGE
    order = FIRST
  []
  [nx_analytic]
    family = LAGRANGE
    order = FIRST
  []
  [ny_analytic]
    family = LAGRANGE
    order = FIRST
  []
  [nz_analytic]
    family = LAGRANGE
    order = FIRST
  []
  [sdf_mesh]
    family = LAGRANGE
    order = FIRST
  []
  [nx_mesh]
    family = LAGRANGE
    order = FIRST
  []
  [ny_mesh]
    family = LAGRANGE
    order = FIRST
  []
  [nz_mesh]
    family = LAGRANGE
    order = FIRST
  []
  [sdf_err]
    family = LAGRANGE
    order = FIRST
  []
  [n_err]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [sdf_analytic]
    type = LevelSetContactorAux
    variable = sdf_analytic
    contactor = sphere_analytic
    quantity = signed_distance
    execute_on = INITIAL
  []
  [nx_analytic]
    type = LevelSetContactorAux
    variable = nx_analytic
    contactor = sphere_analytic
    quantity = normal_x
    execute_on = INITIAL
  []
  [ny_analytic]
    type = LevelSetContactorAux
    variable = ny_analytic
    contactor = sphere_analytic
    quantity = normal_y
    execute_on = INITIAL
  []
  [nz_analytic]
    type = LevelSetContactorAux
    variable = nz_analytic
    contactor = sphere_analytic
    quantity = normal_z
    execute_on = INITIAL
  []
  [sdf_mesh]
    type = LevelSetContactorAux
    variable = sdf_mesh
    contactor = sphere_mesh
    quantity = signed_distance
    execute_on = INITIAL
  []
  [nx_mesh]
    type = LevelSetContactorAux
    variable = nx_mesh
    contactor = sphere_mesh
    quantity = normal_x
    execute_on = INITIAL
  []
  [ny_mesh]
    type = LevelSetContactorAux
    variable = ny_mesh
    contactor = sphere_mesh
    quantity = normal_y
    execute_on = INITIAL
  []
  [nz_mesh]
    type = LevelSetContactorAux
    variable = nz_mesh
    contactor = sphere_mesh
    quantity = normal_z
    execute_on = INITIAL
  []
  [sdf_err]
    type = ParsedAux
    variable = sdf_err
    coupled_variables = 'sdf_analytic sdf_mesh'
    expression = 'abs(sdf_mesh - sdf_analytic)'
    execute_on = INITIAL
  []
  [n_err]
    type = ParsedAux
    variable = n_err
    coupled_variables = 'nx_analytic ny_analytic nz_analytic nx_mesh ny_mesh nz_mesh'
    expression = 'sqrt((nx_mesh-nx_analytic)^2 + (ny_mesh-ny_analytic)^2 + (nz_mesh-nz_analytic)^2)'
    execute_on = INITIAL
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_sdf_err]
    type = NodalExtremeValue
    variable = sdf_err
    value_type = max
    execute_on = INITIAL
  []
  [max_n_err]
    type = NodalExtremeValue
    variable = n_err
    value_type = max
    execute_on = INITIAL
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = INITIAL
  []
[]
