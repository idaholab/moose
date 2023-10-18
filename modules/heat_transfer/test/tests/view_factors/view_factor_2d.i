[GlobalParams]
  view_factor_object_name = unobstructed_vf
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[UserObjects]
  active = 'unobstructed_vf'

  [unobstructed_vf]
    type = UnobstructedPlanarViewFactor
    boundary = 'top left right bottom'
    execute_on = INITIAL
  []

  [vf_study]
    type = ViewFactorRayStudy
    execute_on = initial
    boundary = 'left right bottom top'
    face_order = TENTH
    polar_quad_order = 80
  []

  [rt_vf]
    type = RayTracingViewFactor
    boundary = 'left right bottom top'
    execute_on = INITIAL
    normalize_view_factor = false
    ray_study_name = vf_study
  []
[]

##
## Reference: bottom -> left/right = 0.19098
##            bottom -> top = 0.61803
## Result at spatial order 20, angular order 200 & -r2
##            bottom -> left/right = 0.1911
##            bottom -> top = 0.6177
##
## For convenience, the "view_factor_object_name" for these
## PPs are set in global params for switching between methods
##
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

  [left_bottom]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = bottom
  [../]

  [bottom_left]
    type = ViewFactorPP
    from_boundary = bottom
    to_boundary = left
  []

  [bottom_right]
    type = ViewFactorPP
    from_boundary = bottom
    to_boundary = right
  []

  [bottom_top]
    type = ViewFactorPP
    from_boundary = bottom
    to_boundary = top
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
