[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
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
    names = 'ray0 ray1'
    start_points = '0 0.5 0
                    0 0.1 0'
    end_points = '2.0 0.5 0
                  2.0 0.9 0'
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
  [ray0_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = ray0
  []
  [ray1_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = ray1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Problem]
  coord_type = RZ
[]

[Outputs]
  csv = true
[]
