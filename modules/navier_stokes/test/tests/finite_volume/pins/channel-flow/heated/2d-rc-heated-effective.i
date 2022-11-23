mu = 1
rho = 1
cp = 1
u_inlet = 1
T_inlet = 200
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
  [left]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 3 & x < 6'
    block_id = 1
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = left
    combinatorial_geometry = 'x < 3'
    block_id = 2
  []
  [more-right]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x > 6'
    block_id = 3
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
  []
  [T_fluid]
    type = INSFVEnergyVariable
  []
[]

[AuxVariables]
  [T_solid]
    family = 'MONOMIAL'
    order = 'CONSTANT'
    fv = true
    initial_condition = 100
  []
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
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

  [energy_advection]
    type = PINSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [energy_diffusion_1]
    type = PINSFVEnergyAnisotropicDiffusion
    kappa = 'kappa'
    variable = T_fluid
    porosity = porosity
    block = '1 2'
  []
  [energy_diffusion_2]
    type = PINSFVEnergyAnisotropicDiffusion
    kappa = 'kappa'
    variable = T_fluid
    porosity = porosity
    block = '3'
  []
  [energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_fluid
    is_solid = false
    T_fluid = T_fluid
    T_solid = T_solid
    h_solid_fluid = 'h_cv'
  []
[]

[FVBCs]
  inactive = 'inlet-T-dirichlet'
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
  [inlet-T-dirichlet]
    type = FVDirichletBC
    variable = T_fluid
    value = '${T_inlet}'
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
    function = 0.1
  []
[]

[Materials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv'
    prop_values = '1'
  []
  [functor_constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
  []
  [kappa]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = '1e-3 1e-2 1e-1'
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    rho = ${rho}
    temperature = 'T_fluid'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
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
