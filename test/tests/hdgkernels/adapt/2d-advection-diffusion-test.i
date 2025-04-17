[GlobalParams]
  variable = u
  face_variable = side_u
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
  elem_type = TRI6
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
  []
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
  []
[]

[HDGKernels]
  [advection]
    type = AdvectionIPHDGKernel
    velocity = vel
  []
  [diffusion]
    type = DiffusionIPHDGKernel
    diffusivity = 1
  []
[]

[BCs]
  [inflow]
    type = AdvectionIPHDGPrescribedFluxBC
    boundary = 'left'
    velocity = vel
    prescribed_normal_flux = -1
  []
  [outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    velocity = vel
  []
  [diffusive_influx]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'bottom'
    diffusivity = 1
    prescribed_normal_flux = -1
  []
  [dirichlet]
    type = DiffusionIPHDGDirichletBC
    boundary = 'top'
    diffusivity = 1
    functor = 1
  []
  [zero_diffusive_flux]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'left right'
    diffusivity = 1
    prescribed_normal_flux = 0
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
  initial_steps = 2
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
  exodus = true
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
[]
