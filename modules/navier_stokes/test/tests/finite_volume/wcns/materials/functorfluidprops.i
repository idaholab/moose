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
    type = FVTimeKernel
    variable = u
  []
  [v_time]
    type = FVTimeKernel
    variable = v
  []
  [p_time]
    type = FVTimeKernel
    variable = pressure
  []
  [T_time]
    type = FVTimeKernel
    variable = T
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = FlibeFluidProperties
    []
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
  []
  [drho_dp_var]
  []
  [drho_dT_var]
  []
  [rho_dot_var]
  []
  [cp_var]
  []
  [dcp_dp_var]
  []
  [dcp_dT_var]
  []
  [cp_dot_var]
  []
  [cv_var]
  []
  [mu_var]
  []
  [dmu_dp_var]
  []
  [dmu_dT_var]
  []
  [k_var]
  []
  [dk_dp_var]
  []
  [dk_dT_var]
  []
  [Pr_var]
  []
  [dPr_dp_var]
  []
  [dPr_dT_var]
  []
  [Re_var]
  []
  [dRe_dp_var]
  []
  [dRe_dT_var]
  []
  [Re_h_var]
  []
  [Re_i_var]
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
    mat_prop = 'rho'
    variable = 'rho_var'
    execute_on = 'timestep_begin'
  []
  [drho_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'drho/dpressure'
    variable = 'drho_dp_var'
    execute_on = 'timestep_begin'
  []
  [drho_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'drho/dT_fluid'
    variable = 'drho_dT_var'
    execute_on = 'timestep_begin'
  []
  [drho_dt_out]
    type = ADFunctorElementalAux
    mat_prop = 'drho_dt'
    variable = 'rho_dot_var'
    execute_on = 'timestep_begin'
  []
  [cp_out]
    type = ADFunctorElementalAux
    mat_prop = 'cp'
    variable = 'cp_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dcp/dpressure'
    variable = 'dcp_dp_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dcp/dT_fluid'
    variable = 'dcp_dT_var'
    execute_on = 'timestep_begin'
  []
  [dcp_dt_out]
    type = ADFunctorElementalAux
    mat_prop = 'dcp_dt'
    variable = 'cp_dot_var'
    execute_on = 'timestep_begin'
  []
  [cv_out]
    type = ADFunctorElementalAux
    mat_prop = 'cv'
    variable = 'cv_var'
    execute_on = 'timestep_begin'
  []
  [mu_out]
    type = ADFunctorElementalAux
    mat_prop = 'mu'
    variable = 'mu_var'
    execute_on = 'timestep_begin'
  []
  [dmu_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dmu/dpressure'
    variable = 'dmu_dp_var'
    execute_on = 'timestep_begin'
  []
  [dmu_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dmu/dT_fluid'
    variable = 'dmu_dT_var'
    execute_on = 'timestep_begin'
  []
  [k_out]
    type = ADFunctorElementalAux
    mat_prop = 'k'
    variable = 'k_var'
    execute_on = 'timestep_begin'
  []
  [dk_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dk/dpressure'
    variable = 'dk_dp_var'
    execute_on = 'timestep_begin'
  []
  [dk_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dk/dT_fluid'
    variable = 'dk_dT_var'
    execute_on = 'timestep_begin'
  []
  [Pr_out]
    type = ADFunctorElementalAux
    mat_prop = 'Pr'
    variable = 'Pr_var'
    execute_on = 'timestep_begin'
  []
  [dPr_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dPr/dpressure'
    variable = 'dPr_dp_var'
    execute_on = 'timestep_begin'
  []
  [dPr_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dPr/dT_fluid'
    variable = 'dPr_dT_var'
    execute_on = 'timestep_begin'
  []
  [Re_out]
    type = ADFunctorElementalAux
    mat_prop = 'Re'
    variable = 'Re_var'
    execute_on = 'timestep_begin'
  []
  [dRe_dp_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dRe/dpressure'
    variable = 'dRe_dp_var'
    execute_on = 'timestep_begin'
  []
  [dRe_dT_out]
    type = FunctorMatPropElementalAux
    mat_prop = 'dRe/dT_fluid'
    variable = 'dRe_dT_var'
    execute_on = 'timestep_begin'
  []
  [Re_h_out]
    type = ADFunctorElementalAux
    mat_prop = 'Re_h'
    variable = 'Re_h_var'
    execute_on = 'timestep_begin'
  []
  [Re_i_out]
    type = ADFunctorElementalAux
    mat_prop = 'Re_i'
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
