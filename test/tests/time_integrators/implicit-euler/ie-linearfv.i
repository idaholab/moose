###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of the TimeIntegrator system.
#
# Testing a solution that is second order in space
# and first order in time
#
# @Requirement F1.30
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
  []
[]

[Functions]
  [forcing_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))-(4*t)
  []
  [exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  []
[]

[LinearFVKernels]
  [ie]
    type = LinearFVTimeDerivative
    variable = u
  []
  [diff]
    type = LinearFVDiffusion
    variable = u
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = forcing_fn
  []
[]

[LinearFVBCs]
  [all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = '0 1 2 3'
    functor = exact_fn
  []
[]

[Postprocessors]
  [l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  []
[]

[Executioner]
  type = Transient

  system_names = u_sys
  l_tol = 1e-10

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  # Test of the TimeIntegrator System
  scheme = 'implicit-euler'

  start_time = 0.0
  num_steps = 5
  dt = 0.25
[]

[Outputs]
  exodus = true
[]
