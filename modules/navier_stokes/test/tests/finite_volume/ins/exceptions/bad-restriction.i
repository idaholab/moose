mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

restricted_blocks = '1'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    block = '1 2'
    pressure = pressure
  []
[]

[Mesh]
  parallel_type = 'replicated'
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    ix = '7 7'
    iy = 10
    subdomain_id = '1 2'
  []
  [mid]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    input = mesh
    new_boundary = 'middle'
  []
  [break_top]
    type = PatchSidesetGenerator
    boundary = 'top'
    n_patches = 2
    input = mid
  []
  [break_bottom]
    type = PatchSidesetGenerator
    boundary = 'bottom'
    n_patches = 2
    input = break_top
  []
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = true
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
    block = ${restricted_blocks}
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
    block = ${restricted_blocks}
  []
  [pressure]
    type = INSFVPressureVariable
    block = ${restricted_blocks}
  []
  [temperature]
    type = INSFVEnergyVariable
    block = ${restricted_blocks}
  []
  [scalar]
    type = INSFVScalarFieldVariable
    block = ${restricted_blocks}
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    u = u
    v = v
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
    u = u
    v = v
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

  [energy_advection]
    type = INSFVEnergyAdvection
    variable = temperature
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    u = u
    v = v
    rho = ${rho}
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = 1.1
    variable = temperature
  []
  [energy_loss]
    type = FVBodyForce
    variable = temperature
    value = -0.1
  []

  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    u = u
    v = v
    rho = ${rho}
  []
  [scalar_diffusion]
    type = FVDiffusion
    coeff = 1
    variable = scalar
  []
  [scalar_src]
    type = FVBodyForce
    variable = scalar
    value = 0.1
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
  [top-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'top_0'
    variable = u
    function = 0
  []
  [top-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'top_0'
    variable = v
    function = 0
  []
  [bottom-wall-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom_0'
    variable = u
    mu = ${mu}
    u = u
    v = v
    momentum_component = 'x'
  []
  [bottom-wall-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom_0'
    variable = v
    mu = ${mu}
    u = u
    v = v
    momentum_component = 'y'
  []
  [bottom-wall-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom_0'
    variable = pressure
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'middle'
    variable = pressure
    function = 0
  []
  [inlet_t]
    type = FVDirichletBC
    boundary = 'left'
    variable = temperature
    value = 1
  []
  [outlet_scalar]
    type = FVDirichletBC
    boundary = 'middle'
    variable = scalar
    value = 1
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    temperature = 'temperature'
    rho = ${rho}
    block = ${restricted_blocks}
  []
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
