[GlobalParams]
  view_factor_object_name = rt_vf
[]

[Mesh]
  [with_a_non_planar]
    type = ElementGenerator
    nodal_positions = '-1 -1 0
                       1 -1 0
                       1 1 0
                       -1 1 0
                       -1 -1 2
                       1 -1 2
                       1 1 2
                       -1 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = 'HEX8'
  []
  [sides]
    type = AllSideSetsByNormalsGenerator
    input = with_a_non_planar
  []
  [rename]
    type = RenameBoundaryGenerator
    input = sides
    old_boundary = '1 2 3 4 5 6'
    new_boundary = 'bottom front left back right top'
  []
[]

[UserObjects]
  [vf_study]
    type = ViewFactorRayStudy
    execute_on = INITIAL
    boundary = 'bottom front left back right top'
    face_order = CONSTANT
    polar_quad_order = 3
    azimuthal_quad_order = 200
    face_type = GAUSS
    warn_non_planar = false
  []

  [rt_vf]
    type = RayTracingViewFactor
    boundary = 'bottom front left back right top'
    execute_on = INITIAL
    ray_study_name = vf_study
    normalize_view_factor = false
  []
[]

[RayBCs]
  [vf]
    type = ViewFactorRayBC
    boundary = 'left right front back bottom top'
  []
[]

## For convenience, the "view_factor_object_name" for these
## PPs are set in global params for switching between methods
[Postprocessors]
  [top_bottom]
    type = ViewFactorPP
    from_boundary = top
    to_boundary = bottom
  []
  [top_left]
    type = ViewFactorPP
    from_boundary = top
    to_boundary = left
  []
  [top_right]
    type = ViewFactorPP
    from_boundary = top
    to_boundary = right
  []
  [top_front]
    type = ViewFactorPP
    from_boundary = top
    to_boundary = front
  []
  [top_back]
    type = ViewFactorPP
    from_boundary = top
    to_boundary = back
  []
  [sum]
    type = ParsedPostprocessor
    expression = 'top_back + top_bottom + top_front + top_right + top_left'
    pp_names = 'top_back top_bottom top_front top_right top_left'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  [Quadrature] # higher order quadrature for unobstructed
    order = SECOND
  []
[]

[Outputs]
  csv = true
[]
