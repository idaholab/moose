cp = 1003.5
cv = 716.8
Rs = ${fparse cp - cv}

T_inlet = 145.78
p_inlet = 81800
v_inlet = 363.02
rho_inlet = ${fparse p_inlet / Rs / T_inlet}
e_inlet = ${fparse cv*T_inlet}
rho_E_inlet = ${fparse rho_inlet * (e_inlet + .5*v_inlet*v_inlet)}

T_initial = ${T_inlet}
p_initial = ${fparse 0.01 * p_inlet}
rho_initial = ${fparse p_initial / Rs / T_initial}
e_initial = ${fparse cv*T_initial}
rho_E_initial = ${fparse rho_initial * e_initial}

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
    file = blast_wedge.e
  [../]
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
    initial_condition = ${rho_initial}
    fv = true
  []

  [rho_v_x]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1e-12
    fv = true
  []

  [rho_v_y]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
    fv = true
  []

  [rho_E]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = ${rho_E_initial}
    fv = true
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
    variable = rho_v_x
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v_x
    momentum_component = x
    fp = fp
  []

  # Momentum y conservation
  [momentum_y_time]
    type = FVTimeKernel
    variable = rho_v_y
  []

  [momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v_y
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
  ## inlet conditions
  [mass_inlet]
    type = FVDirichletBC
    variable = rho
    value = ${rho_inlet}
    boundary = left
  []

  [vx_inlet]
    type = FVDirichletBC
    variable = rho_v_x
    value = ${fparse v_inlet * rho_inlet}
    boundary = left
  []

  [vy_inlet]
    type = FVDirichletBC
    variable = rho_v_y
    value = 0
    boundary = left
  []

  [E_inlet]
    type = FVDirichletBC
    variable = rho_E
    value = ${rho_E_inlet}
    boundary = left
  []

  ## free outflow conditions
  [mass_free]
    type = CNSFVHLLCMassImplicitBC
    variable = rho
    fp = fp
    boundary = 'top right'
  []

  [momentum_x_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_v_x
    momentum_component = x
    fp = fp
    boundary = 'top right'
  []

  [momentum_y_outflow]
    type = CNSFVHLLCMomentumImplicitBC
    variable = rho_v_y
    momentum_component = y
    fp = fp
    boundary = 'top right'
  []

  [fluid_energy_outflow]
    type = CNSFVHLLCFluidEnergyImplicitBC
    variable = rho_E
    fp = fp
    boundary = 'top right'
  []

  # wall conditions
  [momentum_x_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v_x
    momentum_component = x
    boundary = 'bottom wedge'
  []

  [momentum_y_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v_y
    momentum_component = y
    boundary = 'bottom wedge'
  []
[]

[AuxVariables]
  [Ma]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Ma_aux]
    type = MachAux
    variable = Ma
    fp = fp
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_v_x
    rhov = rho_v_y
    rho_et = rho_E
  []

  [fluid_props]
    type = GeneralFluidProps
    porosity = 1.00
    characteristic_length = 1.00
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
    petsc_options = '-snes_converged_reason'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.0015
  #dt = .01

  #scheme = explicit-tvd-rk-2
  #solve_type = LINEAR
  [TimeIntegrator]
    type = ActuallyExplicitEuler
  []

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

[Outputs]
  exodus = true
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
