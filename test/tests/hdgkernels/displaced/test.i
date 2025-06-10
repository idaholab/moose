[GlobalParams]
  variable = u
  face_variable = side_u
  diffusivity = 1
  alpha = 6
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD9
  displacements = 'disp_x disp_y'
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

[AuxVariables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
[]

[ICs]
  [disp_x]
    type = FunctionIC
    function = 'x + y'
    variable = disp_x
  []
  [disp_y]
    type = FunctionIC
    function = 'x + y'
    variable = disp_y
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionIPHDGKernel
    use_displaced_mesh = true
  []
[]

[BCs]
  [left]
    type = DiffusionIPHDGDirichletBC
    functor = 0
    boundary = 'left'
    use_displaced_mesh = true
  []
  [right]
    type = DiffusionIPHDGDirichletBC
    functor = 1
    boundary = 'right'
    use_displaced_mesh = true
  []
  [zero_flux]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top bottom'
    prescribed_normal_flux = 0
    use_displaced_mesh = true
  []
[]

[Preconditioning]
  [sc]
    type = StaticCondensation
  []
[]
[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
  []
[]
