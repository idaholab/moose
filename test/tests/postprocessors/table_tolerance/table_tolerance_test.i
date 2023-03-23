# This test verifies that the row tolerance for outputting and displaying postprocessors
# can be controlled via the new_row_tolerance parameter. Normally new rows are only added
# if they are above a given tolerance.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./func]
    type = FunctionAux
    function = 'sin(x + 1e12*t)'
    variable = aux
    execute_on = timestep_begin
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20

  # Very small timestep size
  dt = 1e-13
  dtmin = 1e-13
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./integral]
    type = ElementIntegralVariablePostprocessor
    variable = aux
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Outputs]
  exodus = false
  [./out]
    type = CSV
    new_row_tolerance = 1e-14
  [../]
  [./console]
    type = Console
    new_row_tolerance = 1e-14
  [../]
[]
