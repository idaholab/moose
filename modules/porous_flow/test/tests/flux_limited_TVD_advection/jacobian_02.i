# Checking the Jacobian of Flux-Limited TVD Advection, using flux_limiter_type = superbee
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
  dim = 3
  nx = 2
  xmin = 0
  xmax = 1
  ny = 2
  ymin = -1
  ymax = 2
  bias_y = 1.5
  nz = 2
  zmin = 1
  zmax = 2
  bias_z = 0.8
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x + 0.5 * y - 0.4 * z - 0.1 * sin(x) - 0.1 * cos(y) + 0.2 * exp(-z)'
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
    flux_limiter_type = superbee
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
