###########################################################
# This is a test of the off diagonal jacobian machinery of
# the Discontinuous Galerkin System.
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  elem_type = EDGE2
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]
  [./v]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGCoupledDiffusion
    variable = u
    v = v
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
  petsc_options = '-snes_test_display'
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[ICs]
  [./u]
    type = RandomIC
    min = 0.1
    max = 0.9
    variable = u
  [../]
  [./v]
    type = RandomIC
    min = 0.1
    max = 0.9
    variable = v
  [../]
[]
