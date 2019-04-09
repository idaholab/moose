[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./coupledforce]
    type = CoupledForce
    variable = u
    v = aux_u
  [../]
[]

[AuxVariables]
  [./aux_u]
    family = LAGRANGE
    order = FIRST
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

[Postprocessors]
  [./to_postprocessor]
    type = ElementIntegralVariablePostprocessor
    variable = aux_u
    execute_on = 'transfer nonlinear TIMESTEP_END'
  [../]
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
