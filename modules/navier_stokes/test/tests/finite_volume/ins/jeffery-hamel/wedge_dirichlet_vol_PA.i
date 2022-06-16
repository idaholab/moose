mu=.01
rho=1

# This input file tests whether we can converge to the semi-analytical
# solution for flow in a 2D wedge.
[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'average'
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    # file = wedge_4x6.e
    file = wedge_8x12.e
    # file = wedge_16x24.e
    # file = wedge_32x48.e
    # file = wedge_64x96.e
  []
  [./corner_node]
    # Pin is on the centerline of the channel on the left-hand side of
    # the domain at r=1.  If you change the domain, you will need to
    # update this pin location for the pressure exact solution to
    # work.
    type = ExtraNodesetGenerator
    new_boundary = pinned_node
    coord = '1 0'
    input = file
  [../]
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
  []
  [vel_y]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []

  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu'
    momentum_component = 'x'
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
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu'
    momentum_component = 'y'
  []

  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
[]

[FVBCs]

  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top_wall bottom_wall'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'top_wall bottom_wall'
    function = 0
  []

  [inlet_x]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'inlet'
    function = 1
  []

  [inlet_y]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'inlet'
    function = 0
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = 0
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  num_steps = 5
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-13
  nl_abs_tol = 1e-11
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  exodus = true
[]
