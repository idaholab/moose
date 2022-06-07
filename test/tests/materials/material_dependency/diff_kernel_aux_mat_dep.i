[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = 'diff'
  [../]
[]

[AuxKernels]
  [./error]
    type = ElementLpNormAux
    variable = error
    coupled_variable = u
  [../]
[]

[AuxVariables]
  [./error]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./call_me_mat]
    type = IncrementMaterial
    prop_names = 'diff'
    prop_values = '1'
    block = 0
    outputs = exodus
    output_properties = 'mat_prop'
  [../]
[]

[Executioner]
  type = Steady
# This test counts the number of residual evaluations that
# may slightly change from a PETSc version to another.
# For instance, starts from PETSc-3.8.4, the number of
# residual evaluating is reduced by one in a linear solver
# for each Newton iteration. This change causes this test
# fail. It  better to restrict the test
# count the residual evaluations in the nonlinear level only.
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
[]

[Outputs]
  exodus = true
[]
