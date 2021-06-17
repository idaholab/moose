# Exception test that AdvectiveFluxCalculator is indeed executed on linear
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
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
    execute_on = 'nonlinear timestep_begin timestep_end final initial'
    u = u
    velocity = '0 0 0'
  []
[]


[Preconditioning]
  active = smp
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
  num_steps = 1
  dt = 1
[]
