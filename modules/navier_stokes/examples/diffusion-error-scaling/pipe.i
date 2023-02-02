mu = 8.8871e-4
rho = 997.561
k = '${fparse 0.6203*0.93}'
cp = 4181.72
advected_interp_method = 'average'
velocity_interp_method = 'rc'

U_inlet = 0.0
T_inlet = 300.0
T_cold = 250.0
Nx = 40
Ny = 5

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = false
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
    xmax = 9
    ymin = 0
    ymax = '${fparse 0.5 * 1.0}'
    nx = ${Nx}
    ny = ${Ny}
    #bias_y = '${fparse 1 / 1.2}'
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
    combinatorial_geometry = 'x>0.5 & x<8.5'
    new_sideset_name = 'cooled_wall'
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

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
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

  #inactive = 'u_time v_time T_time'

  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
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
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
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
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [T_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    cp = '${cp}'
    rho = '${rho}'
  []
  [energy_advection]
    type = INSFVEnergyAdvection
    variable = T
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = ${k}
    variable = T
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
  # [inlet_T]
  #   type = FVDirichletBC
  #   boundary = 'inlet'
  #   variable = T
  #   value = 400.0
  # []
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
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    rho = ${rho}
    temperature = 'T'
  []
[]

[Executioner]
  type = Transient
  dt = 1e4
  end_time = 1e5
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-10
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
    start_point = '4.5 ${fparse 0.5/Ny/2.0} 0'
    end_point = '4.5 ${fparse 0.5 - 0.5/Ny/2.0} 0'
    num_points = '${Ny}'
    sort_by = y
    variable = 'T'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
