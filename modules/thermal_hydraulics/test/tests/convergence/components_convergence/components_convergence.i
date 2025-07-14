initial_p = 1e5
initial_T = 500

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.029
  []
  [fp2]
    type = IdealGasFluidProperties
    gamma = 1.5
    molar_mass = 0.04
  []
  [fp_mix]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'fp1 fp2'
  []
[]

[Closures]
  [closures]
    type = FunctorClosures
    properties = 'f_D mass_diffusion_coefficient'
    functors = '0 0.26e-4'
  []
[]

[Functions]
  [initial_mass_fraction_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 5.0'
    y = '0.2 0.4'
  []
[]

[Components]
  [pipe]
    type = FlowChannelGasMix
    position = '0 0 0'
    orientation = '1 0 0'
    length = 10.0
    n_elems = 50
    A = 0.2

    initial_mass_fraction = initial_mass_fraction_fn
    initial_p = ${initial_p}
    initial_T = ${initial_T}
    initial_vel = 0

    fp = fp_mix
    closures = 'closures'

    scaling_factor_rhoEA = 1e-5
  []
  [inlet]
    type = SolidWallGasMix
    input = 'pipe:in'
  []
  [outlet]
    type = SolidWallGasMix
    input = 'pipe:out'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Convergence]
  [comp_conv]
    type = ComponentsConvergence
    min_iterations = 2
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  dt = 1000
  num_steps = 1

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nonlinear_convergence = comp_conv
[]
