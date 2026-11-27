# A linear system v and the nonlinear systems u and w. The source of u is v and the source of w is
# u, so the solution depends on the solve order. All runs solve v, then u, then w.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  nl_sys_names = 'u_sys w_sys'
  linear_sys_names = 'v_sys'
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    solver_sys = u_sys
  []
  [w]
    type = MooseVariableFVReal
    solver_sys = w_sys
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = v_sys
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [source]
    type = FVCoupledForce
    variable = u
    v = v
  []

  [diff_w]
    type = FVDiffusion
    variable = w
    coeff = 1
  []
  [source_w]
    type = FVCoupledForce
    variable = w
    v = u
  []
[]

[LinearFVKernels]
  [diff]
    type = LinearFVDiffusion
    variable = v
  []
  [source]
    type = LinearFVSource
    variable = v
    source_density = 1
  []
[]

[FVBCs]
  [all]
    type = FVDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
  [all_w]
    type = FVDirichletBC
    variable = w
    boundary = 'left right top bottom'
    value = 0
  []
[]

[LinearFVBCs]
  [all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = 'left right top bottom'
    functor = 0
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [w_avg]
    type = ElementAverageValue
    variable = w
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
  []
[]

[Outputs]
  # The Steady and executor runs compare with the same gold file
  file_base = system_solve_out
  csv = true
[]
