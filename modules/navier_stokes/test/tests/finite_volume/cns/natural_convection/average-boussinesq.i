hot_temp=400
cold_temp=273.15
p_initial=1.01e5
T_initial=${cold_temp}
molar_mass=29e-3
R=8.3145
gamma=1.4
rho_initial=${fparse p_initial * molar_mass / (R * T_initial)}
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# No bulk velocity in the domain initially
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
k=25.68e-3
mu=18.23e-6

[GlobalParams]
  gravity = '0.00 -9.81 0.00'
  flux_interp_method = 'average'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1e-2
    ymin = 0
    ymax = 1e-2
    nx = 64
    ny = 64
  []
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
    initial_condition = ${rho_initial}
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_v]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = ${rho_et_initial}
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]


[AuxVariables]
  [vel_x]
    type = MooseVariableFVReal
  []
  [vel_y]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [temperature]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
    execute_on = 'timestep_end'
  []
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = pressure
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = ADMaterialRealAux
    variable = vel_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [vel_y]
    type = ADMaterialRealAux
    variable = vel_y
    property = vel_y
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = NSFVMassFluxAdvection
    variable = rho
    advected_quantity = 1
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = rho
    lambda = lambda
    phi0 = 1.29
  []

  [x_momentum_advection]
    type = NSFVMassFluxAdvection
    variable = rho_u
    advected_quantity = 'vel_x'
  []
  [x_viscous]
    type = FVOrthogonalDiffusion
    variable = rho_u
    coeff = ${mu}
    diffusing_quantity = 'vel_x'
  []
  [x_pressure]
    type = PCNSFVMomentumPressureFlux
    variable = rho_u
    momentum_component = 'x'
  []
  [x_momentum_gravity_source]
    type = PNSFVMomentumGravity
    variable = rho_u
    momentum_component = 'x'
  []

  [y_momentum_advection]
    type = NSFVMassFluxAdvection
    variable = rho_v
    advected_quantity = 'vel_y'
  []
  [y_viscous]
    type = FVOrthogonalDiffusion
    variable = rho_v
    coeff = ${mu}
    diffusing_quantity = 'vel_y'
  []
  [y_pressure]
    type = PCNSFVMomentumPressureFlux
    variable = rho_v
    momentum_component = 'y'
  []
  [y_momentum_gravity_source]
    type = PNSFVMomentumGravity
    variable = rho_v
    momentum_component = 'y'
  []

  [fluid_energy_advection]
    type = NSFVMassFluxAdvection
    variable = rho_et
    advected_quantity = 'ht'
  []
  [fluid_energy_conduction]
    type = FVOrthogonalDiffusion
    variable = rho_et
    coeff = ${k}
    diffusing_quantity = 'T_fluid'
  []
[]

[FVBCs]
  [pressure_x_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'x'
    boundary = 'left right top bottom'
    variable = rho_u
  []
  [pressure_y_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'y'
    boundary = 'left right top bottom'
    variable = rho_v
  []

  [shear_x_walls]
    type = FVOrthogonalBoundaryDiffusion
    function = 0
    variable = rho_u
    diffusing_quantity = 'vel_x'
    coeff = ${mu}
    boundary = 'left right top bottom'
  []
  [shear_y_walls]
    type = FVOrthogonalBoundaryDiffusion
    function = 0
    variable = rho_v
    diffusing_quantity = 'vel_y'
    coeff = ${mu}
    boundary = 'left right top bottom'
  []

  [hot_wall]
    type = FVOrthogonalBoundaryDiffusion
    function = ${hot_temp}
    variable = rho_et
    diffusing_quantity = 'T_fluid'
    coeff = ${k}
    boundary = 'left'
  []
  [cold_wall]
    type = FVOrthogonalBoundaryDiffusion
    function = ${cold_temp}
    variable = rho_et
    diffusing_quantity = 'T_fluid'
    coeff = ${k}
    boundary = 'right'
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_u
    superficial_rhov = rho_v
    fp = fp
    porosity = porosity
  []
  [porosity]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
  []
[]

[Executioner]
  solve_type = NEWTON
  nl_max_its = 20
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
  line_search = 'none'

  type = Steady
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_var_residual_norms = true
[]
