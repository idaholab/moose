mu = 8.8871e-4
rho_solid = 997.561
rho_liquid = 997.561
k_solid = 0.6203
k_liquid = 0.6203
cp_solid = 4181.72
cp_liquid = 4181.72
L = 3e5
T_liquidus = 285
T_solidus = 280

advected_interp_method = 'average'
velocity_interp_method = 'rc'

U_inlet = '${fparse 0.5 * mu / rho_liquid / 0.5}'
T_inlet = 300.0
T_cold = 200.0
Nx = 30
Ny = 5

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
  coord_type = 'RZ'
  rz_coord_axis = 'X'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = '${fparse 0.5 * 1.0}'
    nx = ${Nx}
    ny = ${Ny}
    bias_y = '${fparse 1 / 1.2}'
  []
  [rename1]
    type = RenameBoundaryGenerator
    input = gen
    old_boundary = 'left'
    new_boundary = 'inlet'
  []
  [rename2]
    type = RenameBoundaryGenerator
    input = rename1
    old_boundary = 'right'
    new_boundary = 'outlet'
  []
  [rename3]
    type = RenameBoundaryGenerator
    input = rename2
    old_boundary = 'bottom'
    new_boundary = 'symmetry'
  []
  [rename4]
    type = RenameBoundaryGenerator
    input = rename3
    old_boundary = 'top'
    new_boundary = 'wall'
  []
  [rename5]
    type = ParsedGenerateSideset
    input = rename4
    normal = '0 1 0'
    combinatorial_geometry = 'x>2.0 & x<8.0 & y>0.49999'
    new_sideset_name = 'cooled_wall'
  []
[]

[AuxVariables]
  [U]
    type = MooseVariableFVReal
  []
  [fl]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [density]
    type = MooseVariableFVReal
  []
  [th_cond]
    type = MooseVariableFVReal
  []
  [cp_var]
    type = MooseVariableFVReal
  []
  [darcy_coef]
    type = MooseVariableFVReal
  []
  [fch_coef]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [compute_fl]
    type = NSLiquidFractionAux
    variable = fl
    temperature = T
    T_liquidus = '${T_liquidus}'
    T_solidus = '${T_solidus}'
    execute_on = 'TIMESTEP_END'
  []
  [rho_out]
    type = ADFunctorElementalAux
    functor = 'rho_mixture'
    variable = 'density'
  []
  [th_cond_out]
    type = ADFunctorElementalAux
    functor = 'k_mixture'
    variable = 'th_cond'
  []
  [cp_out]
    type = ADFunctorElementalAux
    functor = 'cp_mixture'
    variable = 'cp_var'
  []
  [darcy_out]
    type = ADFunctorElementalAux
    functor = 'Darcy_coefficient'
    variable = 'darcy_coef'
  []
  [fch_out]
    type = ADFunctorElementalAux
    functor = 'Forchheimer_coefficient'
    variable = 'fch_coef'
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [T]
    type = INSFVEnergyVariable
    initial_condition = '${T_inlet}'
    scaling = 1.0
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = rho_mixture
  []

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = rho_mixture
    momentum_component = 'x'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = rho_mixture
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_friction]
    type = INSFVMomentumFriction
    variable = vel_x
    momentum_component = 'x'
    linear_coef_name = 'Darcy_coefficient'
    quadratic_coef_name = 'Forchheimer_coefficient'
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = rho_mixture
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = rho_mixture
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_friction]
    type = INSFVMomentumFriction
    variable = vel_y
    momentum_component = 'y'
    linear_coef_name = 'Darcy_coefficient'
    quadratic_coef_name = 'Forchheimer_coefficient'
  []

  [T_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    cp = cp_mixture
    rho = rho_mixture
  []
  [energy_advection]
    type = INSFVEnergyAdvection
    variable = T
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = k_mixture
    variable = T
  []
  [energy_source]
    type = NSFVPhaseChangeSource
    variable = T
    L = ${L}
    liquid_fraction = fl
    T_liquidus = ${T_liquidus}
    T_solidus = ${T_solidus}
    rho = 'rho_mixture'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_x
    function = '${U_inlet}'
  []
  [sym_u]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_y
    function = 0
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = vel_y
    function = 0
  []
  [sym_v]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = y
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = 0
  []
  [sym_p]
    type = INSFVSymmetryPressureBC
    boundary = 'symmetry'
    variable = pressure
  []
  [sym_T]
    type = INSFVSymmetryScalarBC
    variable = T
    boundary = 'symmetry'
  []
  [cooled_wall]
    type = FVFunctorDirichletBC
    variable = T
    functor = '${T_cold}'
    boundary = 'cooled_wall'
  []
[]

[Materials]
  [ins_fv]
    type = INSFVEnthalpyMaterial
    rho = rho_mixture
    cp = cp_mixture
    temperature = 'T'
  []
  [eff_cp]
    type = NSFVMixtureMaterial
    phase_2_names = '${cp_solid} ${k_solid} ${rho_solid}'
    phase_1_names = '${cp_liquid} ${k_liquid} ${rho_liquid}'
    prop_names = 'cp_mixture k_mixture rho_mixture'
    phase_1_fraction = fl
  []
  [mushy_zone_resistance]
    type = INSFVMushyPorousFrictionMaterial
    liquid_fraction = 'fl'
    mu = '${mu}'
    rho_l = '${rho_liquid}'
  []
[]

[Executioner]
  type = Transient
  dt = 5e3
  end_time = 1e4
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-8
  nl_max_its = 12
[]

[Postprocessors]
  [average_T]
    type = ElementAverageValue
    variable = T
    outputs = csv
    execute_on = FINAL
  []
[]

[VectorPostprocessors]
  [sat]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0.0 0 0'
    end_point = '10.0 0 0'
    num_points = '${Nx}'
    sort_by = x
    variable = 'T'
    execute_on = FINAL
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
