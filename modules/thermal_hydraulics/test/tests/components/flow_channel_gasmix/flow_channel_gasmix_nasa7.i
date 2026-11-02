# Diffusion of hydrogen in steam, with the properties of both gases from NASA 7-coefficient
# polynomials and the diffusion coefficient from the fluid properties

initial_p = 1e5
initial_T = 1073.15

[FluidProperties]
  [steam]
    type = NASA7IdealGasFluidProperties
    species = H2O
  []
  [hydrogen]
    type = NASA7IdealGasFluidProperties
    species = H2
  []
  [fp_mix]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'steam hydrogen'
  []
[]

[Closures]
  [closures]
    type = FunctorClosures
    properties = 'f_D'
    functors = '0'
  []
[]

[Materials]
  [diffusion_coefficient]
    type = FluidPropertiesBinaryDiffusionCoefMaterial
    vapor_mixture_fp = fp_mix
    block = pipe
  []
[]

[Functions]
  [initial_mass_fraction_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 5.0'
    y = '0.05 0.1'
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
    file_base = flow_channel_gasmix_nasa7
    show = 'mass_fraction p T'
  []
[]
