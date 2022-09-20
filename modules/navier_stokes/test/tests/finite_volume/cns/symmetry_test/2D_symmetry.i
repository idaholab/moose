rho_inside = 1
E_inside = 2.501505578

rho_outside = 0.125
E_outside = 1.999770935

radius = 0.1
angle = 45

[GlobalParams]
  fp = fp
[]

[Debug]
   show_material_props = true
[]

[Mesh]
  [file]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.5
    xmax = 0.5
    nx = 10
    ymin = -0.5
    ymax = 0.5
    ny = 10
  [../]


  [rotate]
    type = TransformGenerator
    vector_value = '${angle} 0 0'
    transform = ROTATE
    input = file
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    allow_imperfect_jacobians = true
  []
[]

[Variables]
  [rho]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]

  [rho_u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e-15
    outputs = none
  []

  [rho_v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e-15
    outputs = none
  []

  [rho_E]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[ICs]
  [rho_ic]
    type = FunctionIC
    variable = rho
    function = 'if (abs(x) < ${radius} & abs(y) < ${radius}, ${rho_inside}, ${rho_outside})'
  []

  [rho_E_ic]
    type = FunctionIC
    variable = rho_E
    function = 'if (abs(x) < ${radius} & abs(y) < ${radius}, ${fparse E_inside * rho_inside}, ${fparse E_outside * rho_outside})'
  []
[]

[FVKernels]
  # Mass conservation
  [mass_time]
    type = FVTimeKernel
    variable = rho
  []

  [mass_advection]
    type = CNSFVMassHLLC
    variable = rho
    fp = fp
  []

  # Momentum x conservation
  [momentum_x_time]
    type = FVTimeKernel
    variable = rho_u
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
    fp = fp
  []

  # Momentum y conservation
  [momentum_y_time]
    type = FVTimeKernel
    variable = rho_v
  []

  [./momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v
    momentum_component = y
  []

  # Fluid energy conservation
  [./fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  []

  [./fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
    fp = fp
  []
[]

[FVBCs]
  ## outflow implicit conditions
  [mass_outflow]
    type = CNSFVHLLCMassImplicitBC
    variable = rho
    fp = fp
    boundary = 'left right top bottom'
  []

  [./momentum_x_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_u
    momentum_component = x
    fp = fp
    boundary = 'left right top bottom'
  []

  [momentum_y_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_v
    momentum_component = y
    fp = fp
    boundary = 'left right top bottom'
  []

  [fluid_energy_outflow]
    type = CNSFVHLLCFluidEnergyImplicitBC
    variable = rho_E
    fp = fp
    boundary = 'left right top bottom'
  []
[]

[AuxVariables]
  [Ma]
    family = MONOMIAL
    order = CONSTANT
  []

  [p]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Ma_aux]
    type = NSMachAux
    variable = Ma
    fluid_properties = fp
    use_material_properties = true
  []

  [p_aux]
    type = ADMaterialRealAux
    variable = p
    property = pressure
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rhov = rho_v
    rho_et = rho_E
  []
  [sound_speed]
    type = SoundspeedMat
    fp = fp
  []
[]

[Postprocessors]
  [cfl_dt]
    type = ADCFLTimeStepSize
    c_names = 'sound_speed'
    vel_names = 'speed'
    CFL = 0.5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.2

  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 2
  []
  l_tol = 1e-8

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl_dt
  []
[]
