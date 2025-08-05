[GlobalParams]
  variable = u
  face_variable = side_u
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
  [side_u]
    order = CONSTANT
    family = SIDE_HIERARCHIC
  []
[]

[HDGKernels]
  [advection]
    type = AdvectionIPHDGKernel
    velocity = vel
  []
[]

[BCs]
  [inflow]
    type = AdvectionIPHDGDirichletBC
    functor = 1
    boundary = 'left'
    velocity = vel
  []
  [outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    velocity = vel
  []
[]

[Materials]
  [vel]
    type = ADGenericConstantVectorMaterial
    prop_names = 'vel'
    prop_values = '1 0 0'
  []
[]

[Adaptivity]
  initial_marker = 'box'
  initial_steps = 1
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '0.5 1 0'
      inside = 'refine'
      outside = 'do_nothing'
    []
  []
[]

[Preconditioning]
  [smp]
    type = StaticCondensation
    petsc_options = '-ksp_view_pmat'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
  line_search = 'none'
[]

[Outputs]
  csv = true
  hide = 'side_u'
[]

[Postprocessors]
  [dofs]
    type = NumDOFs
    system = 'nl'
  []
  [elems]
    type = NumElements
  []
  [u]
    type = ElementAverageValue
    variable = u
  []
  [side_u]
    type = ElementAverageValue
    variable = side_u
  []
[]
