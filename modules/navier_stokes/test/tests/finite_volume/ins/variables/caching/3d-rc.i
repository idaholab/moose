mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

pressure_face_gradient_caching = true
velocity_face_gradient_caching = true
pressure_face_value_caching = true
velocity_face_value_caching = true

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
    nx = 21
    ny = 7
    nz = 7
  []
[]

[Problem]
  fv_bcs_integrity_check = true
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
    cache_face_gradients = ${velocity_face_gradient_caching}
    cache_face_values = ${velocity_face_value_caching}
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    cache_face_gradients = ${velocity_face_gradient_caching}
    cache_face_values = ${velocity_face_value_caching}
  []
  [w]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    cache_face_gradients = ${velocity_face_gradient_caching}
    cache_face_values = ${velocity_face_value_caching}
  []
  [pressure]
    type = INSFVPressureVariable
    cache_face_gradients = ${pressure_face_gradient_caching}
    cache_face_values = ${pressure_face_value_caching}
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    w = w
    mu = ${mu}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    w = w
    mu = ${mu}
    rho = ${rho}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    p = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    w = w
    mu = ${mu}
    rho = ${rho}
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    p = pressure
  []

  [w_advection]
    type = INSFVMomentumAdvection
    variable = w
    advected_quantity = 'rhow'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    w = w
    mu = ${mu}
    rho = ${rho}
  []
  [w_viscosity]
    type = FVDiffusion
    variable = w
    coeff = ${mu}
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = w
    momentum_component = 'z'
    p = pressure
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
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom front back'
    variable = v
  []
  [walls-w]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom front back'
    variable = w
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    w = 'w'
    pressure = 'pressure'
    rho = ${rho}
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
  perf_graph = true
  exodus = true
  csv = true
[]
