[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 5
    ymax = 5
  []
[]

[Variables/v]
  family = MONOMIAL
  order = CONSTANT
  fv = true
[]

[FVKernels/diff]
  type = FVDiffusion
  variable = v
  coeff = coeff
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
  [top_bottom]
    type = FVDirichletBC
    variable = v
    boundary = 'top bottom'
    value = 2
  []
[]

[Materials/diff]
  type = ADGenericFunctorMaterial
  prop_names = 'coeff'
  prop_values = '1'
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Problem]
  kernel_coverage_check = false
[]

[UserObjects/study]
  type = RepeatableRayStudy
  names = 'line_source_ray'
  start_points = '1 1 0'
  end_points = '5 2 0'
  execute_on = PRE_KERNELS # must be set for line sources!
[]

[RayKernels/line_source]
  type = ADLineSourceRayKernel
  variable = v
  value = 5
[]
