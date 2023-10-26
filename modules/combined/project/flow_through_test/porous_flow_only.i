# Input file modified from RobPodgorney version
# - 2D instead of 3D with different resolution.  Effectively this means a 1m height of RobPodgorney aquifer is simulated.  RobPodgorney total mass flux is 2.5kg/s meaning 0.25kg/s is appropriate here
# - Celsius instead of Kelvin
# - no use of PorousFlowPointEnthalpySourceFromPostprocessor since that is not yet merged into MOOSE: a DirichletBC is used instead
# - Use of PorousFlowFullySaturated instead of PorousFlowUnsaturated, and the save_component_rate_in feature to record the change in kg of each species at each node for passing to the Geochem simulation
# - MultiApps and Transfers to transfer information between this simulation and the aquifer_geochemistry.i simulation

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmin = 0
    xmax = 0.0508 #m
    ymin = 0
    ymax = 0.0254 #m
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [f_Al]
    initial_condition = 2.67183E-14
  []
  [f_Ca]
    initial_condition = 2.24309E-05
  []
  [f_Cl]
    initial_condition = 4.61586E-06
  []
  [f_F]
    initial_condition = 1.88594E-07
  []
  [f_Fe]
    initial_condition = 5.53051E-14
  []
  [f_H]
    initial_condition = 0.000402917
  []
  [f_K]
    initial_condition = 3.07681E-06
  []
  [f_Mg]
    initial_condition = 1.51855E-05
  []
  [f_NO3]
    initial_condition = 3.96976E-07
  []
  [f_Na]
    initial_condition = 7.94014E-06
  []
  [f_O2]
    initial_condition = 6.59591E-06
  []
  [f_SO4]
    initial_condition = 1.55825E-05
  []
  [f_SiO2]
    initial_condition = 8.4702E-06
  []
  [f_HCO3]
    initial_condition = 0.024533822
  []
  [porepressure]
    initial_condition = 1E6
  []
[]


[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 5.066E6
    boundary = left
  []
  [constant_outer_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 3.69E+06
    boundary = right
  []
  # [outflow_Al]
  #   type = PorousFlowOutflowBC
  #   boundary = right
  #   include_relperm = false
  #   variable = f_Al
  # []
  # [outflow_Ca]
  #   type = PorousFlowOutflowBC
  #   variable = f_Ca
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_Cl]
  #   type = PorousFlowOutflowBC
  #   variable = f_Cl
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_F]
  #   type = PorousFlowOutflowBC
  #   variable = f_F
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_Fe]
  #   type = PorousFlowOutflowBC
  #   variable = f_Fe
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_H]
  #   type = PorousFlowOutflowBC
  #   variable = f_H
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_HCO3]
  #   type = PorousFlowOutflowBC
  #   variable = f_HCO3
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_K]
  #   type = PorousFlowOutflowBC
  #   variable = f_K
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_Mg]
  #   type = PorousFlowOutflowBC
  #   variable = f_Mg
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_NO3]
  #   type = PorousFlowOutflowBC
  #   variable = f_NO3
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_Na]
  #   type = PorousFlowOutflowBC
  #   variable = f_Na
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_O2]
  #   type = PorousFlowOutflowBC
  #   variable = f_O2
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_SO4]
  #   type = PorousFlowOutflowBC
  #   variable = f_SO4
  #   include_relperm = false
  #   boundary = right
  # []
  # [outflow_SiO2]
  #   type = PorousFlowOutflowBC
  #   variable = f_SiO2
  #   include_relperm = false
  #   boundary = right
  # []
  # [injected_Al]
  #   type = DirichletBC
  #   variable = f_Al
  #   value = 2.67183E-14
  #   boundary = left
  # []
  # [injected_Ca]
  #   type = DirichletBC
  #   variable = f_Ca
  #   value = 2.24309E-05
  #   boundary = left
  # []
  # [injected_Cl]
  #   type = DirichletBC
  #   variable = f_Cl
  #   value = 4.61586E-06
  #   boundary = left
  # []
  # [injected_F]
  #   type = DirichletBC
  #   variable = f_F
  #   value = 1.88594E-07
  #   boundary = left
  # []
  # [injected_Fe]
  #   type = DirichletBC
  #   variable = f_Fe
  #   value = 5.53051E-14
  #   boundary = left
  # []
  # [injected_H]
  #   type = DirichletBC
  #   variable = f_H
  #   value = 0.000402917
  #   boundary = left
  # []
  # [injected_HCO3]
  #   type = DirichletBC
  #   variable = f_HCO3
  #   value = 0.024533822
  #   boundary = left
  # []
  # [injected_K]
  #   type = DirichletBC
  #   variable = f_K
  #   value = 3.07681E-06
  #   boundary = left
  # []
  # [injected_Mg]
  #   type = DirichletBC
  #   variable = f_Mg
  #   value = 1.51855E-05
  #   boundary = left
  # []
  # [injected_NO3]
  #   type = DirichletBC
  #   variable = f_NO3
  #   value = 3.96976E-07
  #   boundary = left
  # []
  # [injected_Na]
  #   type = DirichletBC
  #   variable = f_Na
  #   value = 7.94014E-06
  #   boundary = left
  # []
  # [injected_O2]
  #   type = DirichletBC
  #   variable = f_O2
  #   value = 6.59591E-06
  #   boundary = left
  # []
  # [injected_SO4]
  #   type = DirichletBC
  #   variable = f_SO4
  #   value = 1.55825E-05
  #   boundary = left
  # []
  # [injected_SiO2]
  #   type = DirichletBC
  #   variable = f_SiO2
  #   value = 8.4702E-06
  #   boundary = left
  # []
[]


[Postprocessors]
  [mass_extracted_Al]
    type = NodalSum
    variable = f_Al
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Ca]
    type = NodalSum
    variable = f_Ca
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Cl]
    type = NodalSum
    variable = f_Cl
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_F]
    type = NodalSum
    variable = f_F
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Fe]
    type = NodalSum
    variable = f_Fe
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_H]
    type = NodalSum
    variable = f_H
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_HCO3]
    type = NodalSum
    variable = f_HCO3
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_K]
    type = NodalSum
    variable = f_K
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Mg]
    type = NodalSum
    variable = f_Mg
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_NO3]
    type = NodalSum
    variable = f_NO3
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Na]
    type = NodalSum
    variable = f_Na
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_O2]
    type = NodalSum
    variable = f_O2
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SO4]
    type = NodalSum
    variable = f_SO4
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SiO2]
    type = NodalSum
    variable = f_SiO2
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [darcy_velocity]
    type = SideAverageValue
    variable = darcy_vel_x
    boundary = right
    execute_on = 'initial timestep_end'
  []
  [mass_extracted]
    type = LinearCombinationPostprocessor
    pp_names = 'mass_extracted_Al mass_extracted_Ca mass_extracted_Cl mass_extracted_F mass_extracted_Fe mass_extracted_H mass_extracted_HCO3 mass_extracted_K mass_extracted_Mg mass_extracted_NO3 mass_extracted_Na mass_extracted_O2 mass_extracted_SiO2 mass_extracted_SO4'
    pp_coefs = '1 1 1 1 1 1 1 1 1 1 1 1 1 1'
    execute_on = 'initial timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_begin'
  []
  [volumetric_flow_rate]
    type = VolumetricFlowRate
    vel_x = darcy_vel_x
    boundary = right
    advected_variable = 1010
    execute_on = 'initial timestep_end'
  []

  [delta_HCO3]
    type = ChangeOverTimePostprocessor
    postprocessor = mass_extracted_HCO3
    execute_on = 'initial timestep_end'
  []

  [delta_Ca]
    type = ChangeOverTimePostprocessor
    postprocessor = mass_extracted_Ca
    execute_on = 'initial timestep_end'
  []

  [delta_Na]
    type = ChangeOverTimePostprocessor
    postprocessor = mass_extracted_Na
    execute_on = 'initial timestep_end'
  []

[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1.03E-03
    density0 = 1010
  []
[]

[PorousFlowFullySaturated]
  coupling_type = Hydro
  porepressure = porepressure
  mass_fraction_vars = 'f_Al f_Ca f_Cl f_F f_Fe f_H f_K f_Mg f_Na f_NO3 f_O2 f_SiO2 f_SO4 f_HCO3'
  save_component_rate_in = 'rate_Al rate_Ca rate_Cl rate_F rate_Fe rate_H rate_K rate_Mg rate_Na rate_NO3 rate_O2 rate_SiO2 rate_SO4 rate_HCO3 rate_H2O' # change in kg at every node / dt
  fp = the_simple_fluid
  temperature_unit = Celsius
  pressure_unit = Pa
  multiply_by_density = true
 # stabilization = Full
[]

[AuxVariables]
  [rate_Al]
  []
  [rate_Ca]
  []
  [rate_Cl]
  []
  [rate_F]
  []
  [rate_Fe]
  []
  [rate_H]
  []
  [rate_HCO3]
  []
  [rate_K]
  []
  [rate_Mg]
  []
  [rate_Na]
  []
  [rate_NO3]
  []
  [rate_O2]
  []
  [rate_SiO2]
  []
  [rate_SO4]
  []
  [rate_H2O]
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.025
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '2.96E-17 0 0   0 2.96E-17 0   0 0 2.96E-17'
  []
[]

[Preconditioning]
  active = typically_efficient
  [typically_efficient]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = ' hypre    boomeramg'
  []
  [strong]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      ilu           NONZERO                   2'
  []
  [probably_too_strong]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 31536000 #1 year
  nl_rel_tol = 1E-8
  # steady_state_detection = true
  # steady_state_tolerance = 1E-8
  [TimeStepper]
    type = FunctionDT
    function = 'max(1E6, 0.3 * t)'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[MultiApps]
  [react]
    type = TransientMultiApp
    input_files = basalt_mineral_test.i
    clone_master_mesh = true
    execute_on = 'timestep_end'
  []
[]
[Transfers]
  [changes_due_to_flow]
    type = MultiAppCopyTransfer
    source_variable = 'rate_Al rate_Ca rate_Cl rate_F rate_Fe rate_H rate_HCO3 rate_K rate_Mg rate_Na rate_NO3 rate_O2 rate_SiO2 rate_SO4 rate_H2O porepressure'
    variable = 'pf_rate_Al pf_rate_Ca pf_rate_Cl pf_rate_F pf_rate_Fe pf_rate_H pf_rate_HCO3 pf_rate_K pf_rate_Mg pf_rate_Na pf_rate_NO3 pf_rate_O2 pf_rate_SiO2 pf_rate_SO4 pf_rate_H2O pressure'
    to_multi_app = react
  []
  [massfrac_from_geochem]
    type = MultiAppCopyTransfer
    source_variable = 'massfrac_Al massfrac_Ca massfrac_Cl massfrac_F massfrac_Fe massfrac_H massfrac_HCO3 massfrac_K massfrac_Mg massfrac_Na massfrac_NO3 massfrac_O2 massfrac_SiO2 massfrac_SO4'
    variable = 'f_Al f_Ca f_Cl f_F f_Fe f_H f_HCO3 f_K f_Mg f_Na f_NO3 f_O2 f_SiO2 f_SO4'
    from_multi_app = react
  []
[]
