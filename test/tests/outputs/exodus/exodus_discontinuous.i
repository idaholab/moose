##
# \file exodus/exodus_discontinuous.i
# \example exodus/exodus_discontinuous.i
# Input file for testing discontinuous data output
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./disc_u]
    family = monomial
    order = first
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = disc_u
  [../]
  [./forcing]
    type = BodyForce
    variable = disc_u
    value = 7
  [../]
[]

[DGKernels]
  [./diff_dg]
  type = DGDiffusion
  variable = disc_u
  sigma = 1
  epsilon = 1
  [../]
[]

[Functions]
  [./zero_fn]
    type = ParsedFunction
    expression = 0.0
  [../]
[]

[BCs]
  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = disc_u
    boundary = 'left right top bottom'
    function = zero_fn
    sigma = 1
    epsilon = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  [./exo_out]
    type = Exodus
    discontinuous = true
    file_base = 'exodus_discontinuous_out'
  [../]
[]
