[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  # Since this test prints the number of residual evaluations, its
  # output strongly depends on the number of processors you run it on,
  # and, apparently, the type of Mesh.  To reduce this variability, we
  # limit it to run with SerialMesh only.
  distribution = serial
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./nodes]
    type = NumNodes
    execute_on = 'initial timestep_end'
  [../]

  [./elements]
    type = NumElems
    execute_on = 'initial timestep_end'
  [../]

  [./dofs]
    type = NumDOFs
    execute_on = 'initial timestep_end'
  [../]

  [./residuals]
    type = NumResidualEvaluations
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
  output_initial = true
  print_perf_log = true
[]
