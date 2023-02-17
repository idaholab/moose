# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_v = 4

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = ${inlet_v}
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 2
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = ${outlet_pressure}
  []
  [T]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temp}
  []
[]

[FVKernels]
  [u_time]
    type = FVFunctorTimeKernel
    variable = u
  []
  [v_time]
    type = FVFunctorTimeKernel
    variable = v
  []
  [p_time]
    type = FVFunctorTimeKernel
    variable = pressure
  []
  [T_time]
    type = FVFunctorTimeKernel
    variable = T
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
  []
[]

[Materials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = 'pressure'
    T_fluid = 'T'
    speed = 'velocity_norm'

    # For porous flow
    characteristic_length = 2
    porosity = 'porosity'
  []
[]

[AuxVariables]
  [velocity_norm]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.4
  []
  [rho_var]
    type = MooseVariableFVReal
  []
  [drho_dp_var]
    type = MooseVariableFVReal
  []
  [drho_dT_var]
    type = MooseVariableFVReal
  []
  [rho_dot_var]
    type = MooseVariableFVReal
  []
  [cp_var]
    type = MooseVariableFVReal
  []
  [dcp_dp_var]
    type = MooseVariableFVReal
  []
  [dcp_dT_var]
    type = MooseVariableFVReal
  []
  [cp_dot_var]
    type = MooseVariableFVReal
  []
  [cv_var]
    type = MooseVariableFVReal
  []
  [mu_var]
    type = MooseVariableFVReal
  []
  [dmu_dp_var]
    type = MooseVariableFVReal
  []
  [dmu_dT_var]
    type = MooseVariableFVReal
  []
  [k_var]
    type = MooseVariableFVReal
  []
  [dk_dp_var]
    type = MooseVariableFVReal
  []
  [dk_dT_var]
    type = MooseVariableFVReal
  []
  [Pr_var]
    type = MooseVariableFVReal
  []
  [dPr_dp_var]
    type = MooseVariableFVReal
  []
  [dPr_dT_var]
    type = MooseVariableFVReal
  []
  [Re_var]
    type = MooseVariableFVReal
  []
  [dRe_dp_var]
    type = MooseVariableFVReal
  []
  [dRe_dT_var]
    type = MooseVariableFVReal
  []
  [Re_h_var]
    type = MooseVariableFVReal
  []
  [Re_i_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [speed]
    type = VectorMagnitudeAux
    variable = 'velocity_norm'
    x = u
    y = v
  []

  # To output the functor material properties
  [rho_out]
    type = ADFunctorElementalAux
    functor = 'rho'
    variable = 'rho_var'
    execute_on = 'timestep_begin'
  []
  [drho_dp_out]
    type = FunctorElementalAux
    functor = 'drho/dpressure'
    variable = 'drho_dp_var'
    execute_on = 'timestep_begin'
  []
  [drho_dT_out]
    type = FunctorElementalAux
    functor = 'drho/dT_fluid'
    variable = 'drho_dT_var'
    execute_on = 'timestep_begin'
  []
  [drho_dt_out]
    type = ADFunctorElementalAux
    functor = 'drho_dt'
    variable = 'rho_dot_var'
    execute_on = 'timestep_begin'
  []
  [cp_out]
    type = ADFunctorElementalAux
    functor = 'cp'
    variable = 'cp_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dp_out]
    type = FunctorElementalAux
    functor = 'dcp/dpressure'
    variable = 'dcp_dp_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dT_out]
    type = FunctorElementalAux
    functor = 'dcp/dT_fluid'
    variable = 'dcp_dT_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dt_out]
    type = ADFunctorElementalAux
    functor = 'dcp_dt'
    variable = 'cp_dot_var'
    execute_on = 'timestep_begin'
  []
  [cv_out]
    type = ADFunctorElementalAux
    functor = 'cv'
    variable = 'cv_var'
    execute_on = 'timestep_begin'
  []
  [mu_out]
    type = ADFunctorElementalAux
    functor = 'mu'
    variable = 'mu_var'
    execute_on = 'timestep_begin'
  []
  [dmu_dp_out]
    type = FunctorElementalAux
    functor = 'dmu/dpressure'
    variable = 'dmu_dp_var'
    execute_on = 'timestep_begin'
  []
  [dmu_dT_out]
    type = FunctorElementalAux
    functor = 'dmu/dT_fluid'
    variable = 'dmu_dT_var'
    execute_on = 'timestep_begin'
  []
  [k_out]
    type = ADFunctorElementalAux
    functor = 'k'
    variable = 'k_var'
    execute_on = 'timestep_begin'
  []
  [dk_dp_out]
    type = FunctorElementalAux
    functor = 'dk/dpressure'
    variable = 'dk_dp_var'
    execute_on = 'timestep_begin'
  []
  [dk_dT_out]
    type = FunctorElementalAux
    functor = 'dk/dT_fluid'
    variable = 'dk_dT_var'
    execute_on = 'timestep_begin'
  []
  [Pr_out]
    type = ADFunctorElementalAux
    functor = 'Pr'
    variable = 'Pr_var'
    execute_on = 'timestep_begin'
  []
  [dPr_dp_out]
    type = FunctorElementalAux
    functor = 'dPr/dpressure'
    variable = 'dPr_dp_var'
    execute_on = 'timestep_begin'
  []
  [dPr_dT_out]
    type = FunctorElementalAux
    functor = 'dPr/dT_fluid'
    variable = 'dPr_dT_var'
    execute_on = 'timestep_begin'
  []
  [Re_out]
    type = ADFunctorElementalAux
    functor = 'Re'
    variable = 'Re_var'
    execute_on = 'timestep_begin'
  []
  [dRe_dp_out]
    type = FunctorElementalAux
    functor = 'dRe/dpressure'
    variable = 'dRe_dp_var'
    execute_on = 'timestep_begin'
  []
  [dRe_dT_out]
    type = FunctorElementalAux
    functor = 'dRe/dT_fluid'
    variable = 'dRe_dT_var'
    execute_on = 'timestep_begin'
  []
  [Re_h_out]
    type = ADFunctorElementalAux
    functor = 'Re_h'
    variable = 'Re_h_var'
    execute_on = 'timestep_begin'
  []
  [Re_i_out]
    type = ADFunctorElementalAux
    functor = 'Re_i'
    variable = 'Re_i_var'
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  end_time = 0.1
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
