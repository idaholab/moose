[Mesh]
   [cartesian]
     type = CartesianMeshGenerator
     dim = 3
     dx = '0.55 0.9 0.55'
     dy = '0.55 0.9 0.55'
     dz = '0.75 0.0001 0.25'
     subdomain_id = '0 0 0
                     0 0 0
                     0 0 0
                     0 0 0
                     0 1 0
                     0 0 0
                     0 0 0
                     0 0 0
                     0 0 0'
   []

   [side_set_around_obstruction]
     type = SideSetsBetweenSubdomainsGenerator
     input = cartesian
     primary_block = 1
     paired_block = 0
     new_boundary = 'obstacle'
   []
[]

[UserObjects]
  [view_factor_study]
    type = ViewFactorRayStudy
    execute_on = initial
    boundary = 'left right top bottom front back obstacle'
    face_order = FOURTH
    polar_quad_order = 12
    azimuthal_quad_order = 4
    warn_subdomain_hmax = false
  []

  [view_factor]
    type = RayTracingViewFactor
    boundary = 'left right top bottom front back obstacle'
    execute_on = INITIAL
    ray_study_name = view_factor_study
  []
[]

[RayBCs/viewfactor]
  type = ViewFactorRayBC
  boundary = 'left right top bottom front back obstacle'
[]

# Reference solution for front -> back view factor
# is 0.282833. This result is derived from analytical
# view factors from:
# front -> orthogonal sides around front [left right top bottom]
# front -> obstacle
# front -> everywhere must be sum to 1
#
[Postprocessors]
  [front_back]
    type = ViewFactorPP
    from_boundary = front
    to_boundary = back
    view_factor_object_name = view_factor
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
