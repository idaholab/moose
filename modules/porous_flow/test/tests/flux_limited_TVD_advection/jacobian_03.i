# Checking the Jacobian of Flux-Limited TVD Advection, using flux_limiter_type = vanleer
#
# The initial conditions are u=x.  This means that the argument of the flux limiter is 1, so that
# the flux_limiter=1 everywhere, irrespective of flux_limiter_type (except for 'none').  However
# superbee and minmod are nondifferentiable at this point, so using those flux_limiter_type will
# result in a poor Jacobian
#
# Here we use snes_check_jacobian instead of snes_type=test.  The former just checks the Jacobian for the
# random initial conditions, while the latter checks for u=1 and u=-1
#
# The Jacobian is correct for u=1 and u=-1, but the finite-difference scheme used by snes_type=test gives the
# wrong answer.
# For u=1, the Kuzmin-Turek scheme adds as much antidiffusion as possible, resulting in a central-difference
# version of advection (flux_limiter = 1).  This is correct, and the Jacobian is calculated correctly.
# However, when computing the Jacobian using finite differences, u is increased or decreased at a node.
# This results in that node being at a maximum or minimum, which means no antidiffusion should be added
# (flux_limiter = 0).  This corresponds to a full-upwind scheme.  So the finite-difference computes the
# Jacobian in the full-upwind scenario, which is incorrect (the original residual = 0, after finite-differencing
# the residual comes from the full-upwind scenario).
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 6
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x'
  []
[]

[Kernels]
  [flux]
    type = FluxLimitedTVDAdvection
    variable = u
    advective_flux_calculator = fluo
  []
[]

[UserObjects]
  [fluo]
    type = AdvectiveFluxCalculatorConstantVelocity
    flux_limiter_type = vanleer
    u = u
    velocity = '1 -2 1.5'
  []
[]


[Preconditioning]
  active = smp
  [smp]
    type = SMP
    full = true
    petsc_options = '-snes_check_jacobian'
  []
[]

[Executioner]
  type = Transient
  solve_type = Linear # this is to force convergence even though the nonlinear residual is high: we just care about the Jacobian in this test
  end_time = 1
  num_steps = 1
  dt = 1
[]
