mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'
force_boundary_execution=true

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = true
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1
  []
  [pressure]
    order = CONSTANT
    family = MONOMIAL
    fv = true
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
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
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
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
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
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
  []
[]

[FVBCs]
  [inlet-u]
    type = FVDirichletBC
    boundary = 'left'
    variable = u
    value = 1
  []
  [walls-u]
    type = FVDirichletBC
    boundary = 'top bottom'
    variable = u
    value = 0
  []
  [inlet-and-walls-v]
    type = FVDirichletBC
    boundary = 'left top bottom'
    variable = v
    value = 0
  []
  [outlet_p]
    type = FVDirichletBC
    boundary = 'right'
    variable = pressure
    value = 0
  []
[]

[Materials]
  [rho]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = ${rho}
  []
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]
