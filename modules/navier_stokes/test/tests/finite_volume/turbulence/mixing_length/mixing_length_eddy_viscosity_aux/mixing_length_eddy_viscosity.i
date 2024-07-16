von_karman_const = 0.41

H = 1 #halfwidth of the channel
L = 150

Re = 13700

rho = 1
bulk_u = 1
mu = ${fparse rho * bulk_u * 2 * H / Re}

advected_interp_method='upwind'
velocity_interp_method='rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '0.667 0.333'
    ix = '100'
    iy = '10  1'
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [mixing_len]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [wall_shear_stress]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [wall_yplus]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [eddy_viscosity]
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

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = u
    rho = ${rho}
    momentum_component = 'x'
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
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = u
    rho = ${rho}
    mixing_length = mixing_len
    momentum_component = 'x'
    u = u
    v = v
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = v
    rho = ${rho}
    momentum_component = 'y'
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
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = v
    rho = ${rho}
    mixing_length = mixing_len
    momentum_component = 'y'
    u = u
    v = v
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
[]

[AuxKernels]
  [mixing_len]
    type = WallDistanceMixingLengthAux
    walls = 'top'
    variable = mixing_len
    execute_on = 'initial'
    von_karman_const = ${von_karman_const}
    delta = 0.5
  []
  [turbulent_viscosity]
    type = INSFVMixingLengthTurbulentViscosityAux
    variable = eddy_viscosity
    mixing_length = mixing_len
    u = u
    v = v
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
    function = '0'
  []
  [wall-u]
    type = INSFVWallFunctionBC
    variable = u
    boundary = 'top'
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    momentum_component = x
  []
  [wall-v]
    type = INSFVWallFunctionBC
    variable = v
    boundary = 'top'
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    momentum_component = y
  []
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = u
    u = u
    v = v
    mu = ${mu}
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = v
    u = u
    v = v
    mu = ${mu}
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

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    dt = 1e-3
  []
  nl_abs_tol = 1e-8
  end_time = 1e9
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
