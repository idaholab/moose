[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Variables/u]
[]

[BCs]
  [fixed]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    value = 10
  []
[]

[UserObjects]
  [study]
    type = RepeatableRayStudy
    names = 'ray0'
    start_points = '0 0 0'
    end_points = '1 0 0'
  []
[]

[RayKernels]
  [variable_integral]
    type = VariableIntegralRayKernel
    study = study
    variable = u
  []
[]

[Postprocessors]
  [value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = ray0
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Outputs]
  csv = true
[]
