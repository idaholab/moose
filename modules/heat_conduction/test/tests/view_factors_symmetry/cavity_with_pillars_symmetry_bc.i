[Mesh]
  [cartesian]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.5 0.5 0.5'
    dy = '0.5 0.75 0.5'
    dz = '1.5 0.5'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1

                    1 1 1
                    1 1 1
                    1 1 1'
  []

  [add_obstruction]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 2
    paired_block = 1
    new_boundary = obstruction
    input = cartesian
  []

  [add_new_back]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z) < 1e-10'
    included_subdomain_ids = '1'
    normal = '0 0 -1'
    new_sideset_name = back_2
    input = add_obstruction
  []
[]

[UserObjects]
  [view_factor_study]
    type = ViewFactorRayStudy
    execute_on = initial
    boundary = 'left top bottom front back_2 obstruction'
    face_order = FOURTH
  []

  [view_factor]
    type = RayTracingViewFactor
    boundary = 'left top bottom front back_2 obstruction'
    execute_on = INITIAL
    normalize_view_factor = false
    ray_study_name = view_factor_study
  []
[]

[RayBCs]
  [vf_bc]
    type = ViewFactorRayBC
    boundary = 'left top bottom front back_2 obstruction'
  []

  [symmetry]
    type = ReflectRayBC
    boundary = 'right'
  []
[]

[Postprocessors]
  [left_left]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = left
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
