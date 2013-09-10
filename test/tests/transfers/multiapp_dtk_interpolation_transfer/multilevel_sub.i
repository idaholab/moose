[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  xmax = 4
  ymax = 4
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./subsub]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.3

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  print_linear_residuals = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '1 1 0 3 2 0'
    input_files = multilevel_subsub.i
  [../]
[]

[Transfers]
  [./from_subsub]
    type = MultiAppDTKInterpolationTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = u
    variable = subsub
  [../]
[]

