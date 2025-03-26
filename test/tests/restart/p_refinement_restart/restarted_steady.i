[Mesh]
  [cmg]
    type = FileMeshGenerator
    file = steady_out_cp/LATEST
    skip_partitioning = true
  []
[]

[Problem]
  restart_file_base = steady_out_cp/LATEST
[]

[Variables]
  [u]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxVariables]
  [test][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
[]

[BCs]
  [left_u]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = 0
    epsilon = -1
    sigma = 6
  []
[]

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
