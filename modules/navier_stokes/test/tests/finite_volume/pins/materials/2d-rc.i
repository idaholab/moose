mu=0.01
rho=2000
u_inlet = 1

advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 10
    ny = 6
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
    porosity = porosity
  []
[]

[Variables]
  [u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = ${u_inlet}
  []
  [v]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.5
  []

  [speed_output]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [speed]
    type = ADFunctorElementalAux
    variable = 'speed_output'
    functor = 'speed'
  []
[]

[FVKernels]
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = ${u_inlet}
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = 0
  []

  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = u
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = v
    function = 0
  []

  [symmetry-u]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = u
    u = u
    v = v
    mu = ${mu}
    momentum_component = 'x'
  []
  [symmetry-v]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = v
    u = u
    v = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []

  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.1
  []
[]

[Materials]
  # Testing this object
  [var_mat]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = u
    superficial_vel_y = v
    porosity = porosity
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-11
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [outlet-u]
    type = SideAverageValue
    variable = u
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
