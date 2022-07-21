von_karman_const = 0.41

H = 1 #halfwidth of the channel
L = 150

Re = 100

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '0.667 0.333'
    ix = '200'
    iy = '10  1'
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [mixing_length]
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
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_x
    rho = ${rho}
    mixing_length = mixing_length
    momentum_component = 'x'
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = vel_y
    rho = ${rho}
    mixing_length = mixing_length
    momentum_component = 'y'
    u = vel_x
    v = vel_y
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
[]

[AuxKernels]
  [mixing_length]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_length
    execute_on = 'initial'
    von_karman_const = ${von_karman_const}
    delta = 0.5
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [wall-u]
    type = INSFVWallFunctionBC
    variable = vel_x
    boundary = 'top'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = x
  []
  [wall-v]
    type = INSFVWallFunctionBC
    variable = vel_y
    boundary = 'top'
    u = vel_x
    v = vel_y
    mu = ${mu}
    rho = ${rho}
    momentum_component = y
  []
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = total_viscosity
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = total_viscosity
    momentum_component = y
  []
  [symmetry_pressure]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[Materials]
  [total_viscosity]
    type = MixingLengthTurbulentViscosityMaterial
    u = 'vel_x' #computes total viscosity = mu_t + mu
    v = 'vel_y' #property is called total_viscosity
    mixing_length = mixing_length
    mu = ${mu}
    rho = ${rho}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
