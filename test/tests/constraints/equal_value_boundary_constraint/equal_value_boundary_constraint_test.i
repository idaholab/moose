[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  elem_type = QUAD4
  allow_renumbering = false
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

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = 'left'
    value = 1.0
  [../]
  [./right]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = 'right'
    value = 0.0
  [../]
[]

# Constraint System
[Constraints]
  [./y_top]
    type = EqualValueBoundaryConstraint
    variable = diffused
    primary = '45'    # node on boundary
    secondary = 'top'    # boundary
    penalty = 10e6
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = ''
  petsc_options_value = ''

  line_search = 'none'
[]

[Postprocessors]
  active = ' '

  [./residual]
    type = Residual
  [../]

  [./nl_its]
    type = NumNonlinearIterations
  [../]

  [./lin_its]
    type = NumLinearIterations
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]
