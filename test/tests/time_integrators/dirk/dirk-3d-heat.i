#
# Testing a solution that is second order in space and first order in time
#

[Mesh]
  type = GeneratedMesh
  dim  = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  nx   = 10
  ny   = 10
  nz   = 10
  elem_type = HEX20
[]

[Variables]
  [./u]
    order  = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = exact_fn
    [../]
  [../]
[]

[Functions]

  [./exact_fn]
    type = ParsedFunction
    value = sin(pi*x)*sin(pi*y)*sin(pi*z)*cos(t)
  [../]
  
  [./forcing_fn]
    type = ParsedFunction
    value = sin(pi*x)*sin(pi*y)*sin(pi*z)*(3.0*pi*pi*cos(t)-sin(t))
  [../]
[]

[Kernels]
  [./ie]
    type     = TimeDerivative
    variable = u
  [../]

  [./diff]
    type     = Diffusion
    variable = u
  [../]
  
  [./ffn]
    type     = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]


[]

[BCs]
  [./all]
    type     = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3 4 5'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type     = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'dirk'
  start_time = 0.0
  end_time   = 0.2
  dt         = 0.1
  nl_rel_tol = 1e-7
  solve_type = 'LINEAR'
[]

[Outputs]
  output_initial = false
  exodus         = true
  print_perf_log = false
[]
