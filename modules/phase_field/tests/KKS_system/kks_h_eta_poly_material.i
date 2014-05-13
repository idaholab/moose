[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 2
[]

[Functions]
  [./ramp_func]
    type = ParsedFunction
    value = 'x'
  [../]
[]

[Variables]
  # order parameter 1
  [./eta1]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ramp_func
    [../]
  [../]

  # order parameter 2
  [./eta2]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ramp_func
    [../]
  [../]
[]

[Materials]
  [./h_eta1]
    type = KKSHEtaPolyMaterial
    block = 0
    h_order = SIMPLE
    eta = eta1
    outputs = exodus
  [../]

  [./h_eta2]
    type = KKSHEtaPolyMaterial
    block = 0
    h_order = HIGH
    eta = eta2
    outputs = exodus
  [../]
[]

[Kernels]
  [./eta1diff]
    type = Diffusion
    variable = eta1
  [../]

  [./eta2diff]
    type = Diffusion
    variable = eta2
  [../]
[]

[Executioner]
  type = Transient
  scheme = crank-nicolson

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'
  l_max_its = 30
  nl_max_its = 20
  start_time = 0.0
  num_steps = 2
  dt = 50.0
[]

[Outputs]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]

