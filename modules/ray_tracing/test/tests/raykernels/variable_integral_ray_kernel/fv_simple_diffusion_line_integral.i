[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 10
  ymax = 10
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
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
  [top_bottom]
    type = FVDirichletBC
    variable = v
    boundary = 'top bottom'
    value = 1
  []
[]

[Materials/diff]
  type = ADGenericFunctorMaterial
  prop_names = 'coeff'
  prop_values = '1'
[]


[UserObjects/study]
  type = RepeatableRayStudy
  names = 'diag
           right_up'
  start_points = '0 0 0
                  10 0 0'
  end_points = '10 10 0
                10 10 0'
[]

[RayKernels/v_integral]
  type = VariableIntegralRayKernel
  study = study
  variable = v
[]

[Postprocessors]
  [diag_line_integral]
    type = RayIntegralValue
    ray_kernel = v_integral
    ray = diag
  []
  [right_up_line_integral]
    type = RayIntegralValue
    ray_kernel = v_integral
    ray = right_up
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  exodus = false
[]
