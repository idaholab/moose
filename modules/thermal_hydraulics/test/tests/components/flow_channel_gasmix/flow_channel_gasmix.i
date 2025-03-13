initial_p = 1e5
initial_T = 500

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.029
    T_c = 132.63
    rho_c = 99.55884676
    e_c = 9.5064379593e+04
  []
  [fp2]
    type = IdealGasFluidProperties
    gamma = 1.5
    molar_mass = 0.04
    T_c = 132.63
    rho_c = 99.55884676
    e_c = 9.5064379593e+04
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

[Executioner]
  type = Transient
  scheme = bdf2

  dt = 1000
  num_steps = 5

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  [exodus]
    type = Exodus
    file_base = flow_channel_gasmix
    show = 'mass_fraction p T'
  []
[]
