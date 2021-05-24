rho_left = 1
E_left = 2.501505578
u_left = 1e-15

rho_right = 0.125
E_right = 1.999770935
u_right = 1e-15

x_sep = 35

[GlobalParams]
  fp = fp
[]

[Mesh]
  [./cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '40 20'
    ix = '200 100'
    dy = '1 20  2  20 1'
    iy = '4 100 10 100 4'
    subdomain_id = '0 0
                    0 1
                    1 1
                    0 1
                    0 0'
  [../]

  [./wall]
    type = SideSetsBetweenSubdomainsGenerator
    input = cartesian
    master_block = 1
    paired_block = 0
    new_boundary = 'wall'
  [../]

  [./delete]
    type = BlockDeletionGenerator
    input = wall
    block_id = 0
  [../]
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      allow_imperfect_jacobians = true
    [../]
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./rho]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
  [./rho_u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
  [./rho_v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
  [./rho_E]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
[]

[AuxVariables]
  [./Ma]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./p]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./v_norm]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./temperature]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./Ma_aux]
    type = MachAux
    variable = Ma
    fp = fp
  [../]

  [./p_aux]
    type = ADMaterialRealAux
    variable = p
    property = p
  [../]

  [./v_norm_aux]
    type = ADMaterialRealAux
    variable = v_norm
    property = speed
  [../]

  [./temperature_aux]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
  [../]
[]

[FVKernels]
  [./mass_time]
    type = FVTimeKernel
    variable = rho
  [../]

  [./mass_advection]
    type = CNSFVMassHLLC
    variable = rho
  [../]

  [./momentum_x_time]
    type = FVTimeKernel
    variable = rho_u
  [../]

  [./momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
  [../]

  [./momentum_y_time]
    type = FVTimeKernel
    variable = rho_v
  [../]

  [./momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v
    momentum_component = y
  [../]

  [./fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  [../]

  [./fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
  [../]
[]

[FVBCs]
  #[./zero_vel]
  #  type = FVDirichletBC
  #  variable = 'rho_u'
  #  value = 0
  #  boundary = 'left right'
  #[../]

  [./mom_x_pressure]
    type = CNSFVMomImplicitPressureBC
    variable = rho_u
    momentum_component = x
    boundary = 'left right wall'
  [../]

  [./mom_y_pressure]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v
    momentum_component = y
    boundary = 'wall'
  [../]
[]

[ICs]
  [./rho_ic]
    type = FunctionIC
    variable = rho
    function = 'if (x < ${x_sep}, ${rho_left}, ${rho_right})'
  [../]

  [./rho_u_ic]
    type = FunctionIC
    variable = rho_u
    function = 'if (x < ${x_sep}, ${fparse rho_left * u_left}, ${fparse rho_right * u_right})'
  [../]

  [./rho_E_ic]
    type = FunctionIC
    variable = rho_E
    function = 'if (x < ${x_sep}, ${fparse E_left * rho_left}, ${fparse E_right * rho_right})'
  [../]
[]

[Materials]
  [./var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rhov = rho_v
    rho_et = rho_E
    fp = fp
  [../]

  [./fluid_props]
    type = GeneralFluidProps
    porosity = 1
    pebble_diameter = 0.06
    fp = fp
  [../]

  [./sound_speed]
    type = SoundspeedMat
    fp = fp
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]
[]

#[Adaptivity]
#  initial_steps = 0
#  max_h_level = 2
#  marker = mach
#  [./Markers]
#    [./mach]
#      type = ValueThresholdMarker
#      coarsen = 0.3
#      variable = Ma
#      refine = 0.85
#    [../]
#  [../]
#[]

[Postprocessors]
  [./cfl_dt]
    type = ADCFLTimeStepSize
    c_names = 'sound_speed'
    vel_names = 'speed'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 100
  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 2
  []
  l_tol = 1e-8

  [./TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl_dt
  [../]
[]

[Outputs]
  exodus = true
[]
