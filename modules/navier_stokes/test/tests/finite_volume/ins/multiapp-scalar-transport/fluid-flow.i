mu=1
rho=1

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method='average'
  velocity_interp_method='rc'
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

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [ax_out]
    type = MooseVariableFVReal
  []
  [ay_out]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [ax_out]
    type = ADFunctorElementalAux
    functor = ax
    variable = ax_out
    execute_on = timestep_end
  []
  [ay_out]
    type = ADFunctorElementalAux
    functor = ay
    variable = ay_out
    execute_on = timestep_end
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
    variable = u
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
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = u
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = v
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
[]

[MultiApps]
  [scalar]
    type = FullSolveMultiApp
    execute_on = 'timestep_end'
    input_files = 'scalar-transport.i'
  []
[]

[Transfers]
  [ax]
    type = MultiAppCopyTransfer
    source_variable = ax_out
    variable = ax
    execute_on = 'timestep_end'
    to_multi_app = 'scalar'
  []
  [ay]
    type = MultiAppCopyTransfer
    source_variable = ay_out
    variable = ay
    execute_on = 'timestep_end'
    to_multi_app = 'scalar'
  []
  [u]
    type = MultiAppCopyTransfer
    source_variable = u
    variable = u
    execute_on = 'timestep_end'
    to_multi_app = 'scalar'
  []
  [v]
    type = MultiAppCopyTransfer
    source_variable = v
    variable = v
    execute_on = 'timestep_end'
    to_multi_app = 'scalar'
  []
  [pressure]
    type = MultiAppCopyTransfer
    source_variable = pressure
    variable = pressure
    execute_on = 'timestep_end'
    to_multi_app = 'scalar'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
