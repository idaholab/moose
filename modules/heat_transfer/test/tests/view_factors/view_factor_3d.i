[GlobalParams]
  view_factor_object_name = unobstructed_vf
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  nx = 2
  ny = 2
  nz = 2
[]

[UserObjects]
  active = 'unobstructed_vf'

  [unobstructed_vf]
    type = UnobstructedPlanarViewFactor
    boundary = 'left right front back bottom top'
    execute_on = INITIAL
  []

  [vf_study]
    type = ViewFactorRayStudy
    execute_on = INITIAL
    boundary = 'left right front back bottom top'
    face_order = FIFTH
    polar_quad_order = 12
    azimuthal_quad_order = 4
  []

  [rt_vf]
    type = RayTracingViewFactor
    boundary = 'left right front back bottom top'
    execute_on = INITIAL
    ray_study_name = vf_study
    normalize_view_factor = false
  []
[]

## For convenience, the "view_factor_object_name" for these
## PPs are set in global params for switching between methods
[Postprocessors]
  [left_right]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = right
  []

  [left_top]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = top
  []

  [left_back]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = back
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
