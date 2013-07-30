[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  elem_type = QUAD4
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]


[BCs]
  [./left]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 1.0
  [../]
  [./right]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 0.0
  [../]
[]

[Constraints]
  [./y_top]
    type = EqualValueBoundaryConstraint
    variable = diffused
    master = '45'    # node on boundary
    slave = 'top'    # boundary
    penalty = 10e6
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  petsc_options_iname = '-snes_linesearch_type'
  petsc_options_value = 'basic'
[]

[Postprocessors]
  active = ' '

  [./residual]
    type = PrintResidual
  [../]

  [./nl_its]
    type = PrintNumNonlinearIters
  [../]

  [./lin_its]
    type = PrintNumLinearIters
  [../]
[]

[Output]
  file_base = out
  exodus = true
  perf_log = true
[]
