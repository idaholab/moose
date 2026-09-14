# Temporal convergence of LinearFVTimeDerivative when its multiplier varies in time.
#
# The companion linearfv.i holds the multiplier at its default of one, which cannot distinguish
# d(cu)/dt from c du/dt: where c does not move, every state of it is the same number and the two
# operators are identical. This input moves it.
#
#   u = t^3 x y        c = 1 + t/2
#
# u is bilinear so lap(u) = 0 and the diffusion term contributes nothing to the balance; the
# transient is the only operator the measured order can be attributed to. The forcing is the
# conservative form of the transient,
#
#   d(c u)/dt = c' u + c u' = x y ( 2 t^3 + 3 t^2 ) ,
#
# so the manufactured solution is recovered only if that is what the kernel assembles.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 32
  ny = 32
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[LinearFVKernels]
  [time]
    type = LinearFVTimeDerivative
    variable = u
    factor = c
  []
  [diff]
    type = LinearFVDiffusion
    variable = u
  []
  [force]
    type = LinearFVSource
    variable = u
    source_density = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 't^3*x*y'
  []
  [c]
    type = ParsedFunction
    expression = '1 + t/2'
  []
  [force]
    type = ParsedFunction
    expression = 'x*y*(2*t^3 + 3*t^2)'
  []
[]

[LinearFVBCs]
  [all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    functor = exact
    variable = u
    boundary = 'left right top bottom'
  []
[]

[Executioner]
  type = Transient
  system_names = u_sys
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  dt = 0.25
  end_time = 4.0
  scheme = 'bdf2'
[]

[Postprocessors]
  [L2u]
    type = ElementL2Error
    function = exact
    variable = u
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
