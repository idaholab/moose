stagnation_pressure = 1
stagnation_temperature = 1

[GlobalParams]
  fp = fp
[]

[Problem]
  kernel_coverage_check = false
[]

[Debug]
   show_material_props = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = supersonic_nozzle.e
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
      allow_imperfect_jacobians = true
    []
  []
[]

[Variables]
  [rho]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.0034
  []

  [rho_u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e-4
    outputs = none
  []

  [rho_v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    outputs = none
  []

  [rho_E]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 2.5
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

  [momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v
    momentum_component = y
  []

  # Fluid energy conservation
  [fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
    fp = fp
  []
[]

[FVBCs]
  ## inflow stagnation boundaries
  [mass_stagnation_inflow]
    type = CNSFVHLLCMassStagnationInletBC
    variable = rho
    fp = fp
    stagnation_pressure = ${stagnation_pressure}
    stagnation_temperature = ${stagnation_temperature}
    boundary = left
  []

  [momentum_x_stagnation_inflow]
    type = CNSFVHLLCMomentumStagnationInletBC
    variable = rho_u
    momentum_component = x
    fp = fp
    stagnation_pressure = ${stagnation_pressure}
    stagnation_temperature = ${stagnation_temperature}
    boundary = left
  []

  [momentum_y_stagnation_inflow]
    type = CNSFVHLLCMomentumStagnationInletBC
    variable = rho_v
    momentum_component = y
    fp = fp
    stagnation_pressure = ${stagnation_pressure}
    stagnation_temperature = ${stagnation_temperature}
    boundary = left
  [../]

  [fluid_energy_stagnation_inflow]
    type = CNSFVHLLCFluidEnergyStagnationInletBC
    variable = rho_E
    fp = fp
    stagnation_pressure = ${stagnation_pressure}
    stagnation_temperature = ${stagnation_temperature}
    boundary = left
  []

  ## outflow implicit conditions
  [mass_outflow]
    type = CNSFVHLLCMassImplicitBC
    variable = rho
    fp = fp
    boundary = right
  []

  [momentum_x_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_u
    momentum_component = x
    fp = fp
    boundary = right
  []

  [momentum_y_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_v
    momentum_component = y
    fp = fp
    boundary = right
  []

  [fluid_energy_outflow]
    type = CNSFVHLLCFluidEnergyImplicitBC
    variable = rho_E
    fp = fp
    boundary = right
  []

  # wall conditions
  [momentum_x_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_u
    momentum_component = x
    boundary = wall
  []

  [momentum_y_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v
    momentum_component = y
    boundary = wall
  []
[]

[AuxVariables]
  [Ma]
    family = MONOMIAL
    order = CONSTANT
  []

  [Ma_layered]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[UserObjects]
  [layered_Ma_UO]
    type = LayeredAverage
    variable = Ma
    num_layers = 100
    direction = x
  []
[]

[AuxKernels]
  [Ma_aux]
    type = MachAux
    variable = Ma
    fp = fp
  []

  [Ma_layered_aux]
    type = SpatialUserObjectAux
    variable = Ma_layered
    user_object = layered_Ma_UO
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarMaterial
    rho = rho
    rhou = rho_u
    rhov = rho_v
    rho_et = rho_E
  []

  [fluid_props]
    type = GeneralFluidProps
    porosity = 1
    characteristic_length = 1
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

  [outflow_Ma]
    type = SideAverageValue
    variable = Ma
    boundary = right
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    petsc_options = '-snes_converged_reason'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.1

  scheme = explicit-tvd-rk-2
  solve_type = LINEAR

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl_dt
  []
  l_tol = 1e-4

  #l_max_its = 60
  #nl_max_its = 40
  #nl_rel_tol = 1e-6
  #nl_abs_tol = 1e-6
[]

[VectorPostprocessors]
  [Ma_layered]
    type = LineValueSampler
    variable = Ma_layered
    start_point = '0 0 0'
    end_point = '10 0 0'
    num_points = 100
    sort_by = x
  []
[]

[Outputs]
  exodus = true
  #[csv]
  #  type = CSV
  #  execute_on = 'FINAL'
  #[]
[]
