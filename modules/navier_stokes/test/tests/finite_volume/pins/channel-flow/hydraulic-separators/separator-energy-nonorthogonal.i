# This test is designed to check for energy conservation
# in separated channels which are described using a nonorthogonal mesh.
# The two inlet temperatures should be preserved at the outlets.

rho=1.1
mu=0.6
k=2.1
cp=5.5
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [file]
    type = FileMeshGenerator
    file = diverging.msh
  []
  [mirror]
    type = SymmetryTransformGenerator
    input = file
    mirror_point = "0 0 0"
    mirror_normal_vector = "1 0 0"
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'file mirror'
    stitch_boundaries_pairs = 'left left'
  []
  [subdomain1]
    type = ParsedSubdomainMeshGenerator
    input = stitch
    combinatorial_geometry = 'x > 0'
    block_id = 1
  []
  [subdomain2]
    type = ParsedSubdomainMeshGenerator
    input = subdomain1
    combinatorial_geometry = 'x < 0'
    block_id = 2
  []
  [separator]
    type = ParsedGenerateSideset
    input = subdomain2
    combinatorial_geometry = 'x > -0.00001 & x < 0.00001'
    replace = true
    new_sideset_name = separator
  []
  [inlet-1]
    type = ParsedGenerateSideset
    input = separator
    combinatorial_geometry = 'y < 0.00001 & x < 0'
    replace = true
    new_sideset_name = inlet-1
  []
  [inlet-2]
    type = ParsedGenerateSideset
    input = inlet-1
    combinatorial_geometry = 'y < 0.00001 & x > 0'
    replace = true
    new_sideset_name = inlet-2
  []
  [outlet-1]
    type = ParsedGenerateSideset
    input = inlet-2
    combinatorial_geometry = 'y > 20.999999 & x < 0'
    replace = true
    new_sideset_name = outlet-1
  []
  [outlet-2]
    type = ParsedGenerateSideset
    input = outlet-1
    combinatorial_geometry = 'y > 20.999999 & x > 0'
    replace = true
    new_sideset_name = outlet-2
  []
  uniform_refine = 1
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  porosity = porosity
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
  []
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 0.1
  []
  [pressure]
    type = BernoulliPressureVariable
    u = superficial_vel_x
    v = superficial_vel_y
    rho = ${rho}
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 300
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
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    momentum_component = 'x'
    mu = ${mu}
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    pressure = pressure
    momentum_component = 'x'
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    momentum_component = 'y'
    mu = ${mu}
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet-1 inlet-2'
    variable = superficial_vel_x
    function = '0.0'
  []
  [inlet-v-1]
    type = INSFVInletVelocityBC
    boundary = 'inlet-1'
    variable = superficial_vel_y
    function = 0.1
  []
  [inlet-v-2]
    type = INSFVInletVelocityBC
    boundary = 'inlet-2'
    variable = superficial_vel_y
    function = 0.2
  []

  [inlet-T-1]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'inlet-1'
    value = 310
  []
  [inlet-T-2]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'inlet-2'
    value = 350
  []

  [walls-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = superficial_vel_x
    momentum_component = 'x'
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  [separator-u]
    type = INSFVVelocityHydraulicSeparatorBC
    boundary = 'separator'
    variable = superficial_vel_x
    momentum_component = 'x'
  []

  [separator-v]
    type = INSFVVelocityHydraulicSeparatorBC
    boundary = 'separator'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  [separator-p]
    type = INSFVPressureHydraulicSeparatorBC
    boundary = 'separator'
    variable = pressure
  []

  [separator-T]
    type = INSFVEnergyHydraulicSeparatorBC
    boundary = 'separator'
    variable = T_fluid
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet-2 outlet-1'
    variable = pressure
    function = 0.4
  []
[]

[FunctorMaterials]
  [porosity-1]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '1.0'
    block = '1'
  []
  [porosity-2]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = '2'
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = superficial_vel_x
    superficial_vel_y = superficial_vel_y
    porosity = porosity
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
    cp = ${cp}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'
  line_search = 'none'
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [outlet_T1]
    type = SideAverageValue
    variable = 'T_fluid'
    boundary = 'outlet-1'
  []
  [outlet_T2]
    type = SideAverageValue
    variable = 'T_fluid'
    boundary = 'outlet-2'
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
