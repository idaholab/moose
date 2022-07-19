# Fluid properties
mu = 1
rho = 1
cp = 1
k = 1e-3

# Solid properties
cp_s = 2
rho_s = 4
k_s = 1e-2
h_fs = 10

# Operating conditions
u_inlet = 1
T_inlet = 200
p_outlet = 10
top_side_temperature = 150

# Numerical scheme
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
    porosity = porosity
  []
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = ${u_inlet}
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${p_outlet}
  []
  [T_fluid]
    type = INSFVEnergyVariable
  []
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 100
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
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

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = superficial_vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = superficial_vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
  []

  [energy_time]
    type = PINSFVEnergyTimeDerivative
    variable = T_fluid
    cp = ${cp}
    rho = ${rho}
    is_solid = false
    porosity = porosity
  []
  [energy_advection]
    type = PINSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [energy_diffusion]
    type = PINSFVEnergyDiffusion
    variable = T_fluid
    k = ${k}
    porosity = porosity
  []
  [energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_fluid
    is_solid = false
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = 'h_cv'
  []

  [solid_energy_time]
    type = PINSFVEnergyTimeDerivative
    variable = T_solid
    cp = ${cp_s}
    rho = ${rho_s}
    is_solid = true
    porosity = porosity
  []
  [solid_energy_diffusion]
    type = FVDiffusion
    variable = T_solid
    coeff = ${k_s}
  []
  [solid_energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    is_solid = true
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = 'h_cv'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_x
    function = ${u_inlet}
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_y
    function = 0
  []
  [inlet-T]
    type = FVNeumannBC
    variable = T_fluid
    value = '${fparse u_inlet * rho * cp * T_inlet}'
    boundary = 'left'
  []

  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_y
    function = 0
  []
  [heated-side]
    type = FVDirichletBC
    boundary = 'top'
    variable = 'T_solid'
    value = ${top_side_temperature}
  []

  [symmetry-u]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_x
    u = superficial_vel_x
    v = superficial_vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [symmetry-v]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_y
    u = superficial_vel_x
    v = superficial_vel_y
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
    function = ${p_outlet}
  []
[]

[Materials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv'
    prop_values = '${h_fs}'
  []
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    rho = ${rho}
    temperature = 'T_fluid'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12

  end_time = 1.5
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
    variable = superficial_vel_x
    boundary = 'right'
  []
  [outlet-temp]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'right'
  []
  [solid-temp]
    type = ElementAverageValue
    variable = T_solid
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
