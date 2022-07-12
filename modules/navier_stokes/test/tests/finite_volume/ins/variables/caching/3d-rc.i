mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

pressure_cell_gradient_caching = true
velocity_cell_gradient_caching = true

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    zmin = -1
    zmax = 1
    nx = 15
    ny = 5
    nz = 5
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    w = w
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
    cache_cell_gradients = ${velocity_cell_gradient_caching}
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    cache_cell_gradients = ${velocity_cell_gradient_caching}
  []
  [w]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    cache_cell_gradients = ${velocity_cell_gradient_caching}
  []
  [pressure]
    type = INSFVPressureVariable
    cache_cell_gradients = ${pressure_cell_gradient_caching}
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []

  [w_advection]
    type = INSFVMomentumAdvection
    variable = w
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'z'
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = w
    mu = ${mu}
    momentum_component = 'z'
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = w
    momentum_component = 'z'
    pressure = pressure
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = 0
  []
  [inlet-w]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = w
    function = 0
  []

  [walls-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom front back'
    variable = u
    momentum_component = 'x'
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom front back'
    variable = v
    momentum_component = 'y'
  []
  [walls-w]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom front back'
    variable = w
    momentum_component = 'z'
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
[]

[Postprocessors]
  [physical]
    type = MemoryUsage
    mem_type = physical_memory
    value_type = total
    # by default MemoryUsage reports the peak value for the current timestep
    # out of all samples that have been taken (at linear and non-linear iterations)
    execute_on = 'INITIAL TIMESTEP_END NONLINEAR LINEAR'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'asm      100               '
  line_search = 'none'
  nl_abs_tol = 1e-8
[]

[Outputs]
  hide = 'physical'
  perf_graph = true
  exodus = true
[]
