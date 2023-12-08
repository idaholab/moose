# geometry
# 8in(0.2032) x 1.5in(0.0381) Cylinder
radius = 0.01905 #m
length = 0.2032 #m
# initial condition
inlet_pressure = 1.78e6
outlet_pressure = 1.63e6
domain_porosity = 0.127
domain_permeability = 3.625E-17 # 3D 7.96E-20
Al_mass = 6.147940e-07
Ca_mass = 2.272036e-05
Cl_mass = 4.537986e-06
F_mass =  1.875223e-07
Fe_mass =  2.356492e-12
H_mass = 3.392909e-04
K_mass =  3.127768e-06
Mg_mass =  1.549918e-05
NO3_mass = 3.947198e-07
Na_mass = 8.083672e-06
O2_mass = 6.412920e-06
SO4_mass = 1.513277e-05
SiO2_mass = 8.907870e-06
HCO3_mass = 2.068976e-02
porepressure = 1.78e6

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]


[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 80
    xmin = 0
    xmax = ${radius} #m
    ymin = 0
    ymax = ${length} #m
  []
  [rename]
    type = RenameBoundaryGenerator
    input = gen
    old_boundary = '0 1 2 3'
    new_boundary = 'inlet right_wall outlet left_wall'
  []
  coord_type = RZ
  # [2D]
  #   type = ConcentricCircleMeshGenerator
  #   num_sectors = 6
  #   radii = ${radius}
  #   rings = 4
  #   has_outer_square = false
  #   preserve_volumes = false
  # []
  # [3D]
  #   type = MeshExtruderGenerator
  #   extrusion_vector = '0 0 ${length}'
  #   input = 2D
  #   num_layers = 80
  #   bottom_sideset = 'left'
  #   top_sideset = 'right'
  # []
[]

[Variables]
  [f_Al]
    initial_condition = ${Al_mass}
  []
  [f_Ca]
    initial_condition = ${Ca_mass}
  []
  [f_Cl]
    initial_condition = ${Cl_mass}
  []
  [f_F]
    initial_condition = ${F_mass}
  []
  [f_Fe]
    initial_condition = ${Fe_mass}
  []
  [f_H]
    initial_condition = ${H_mass}
  []
  [f_K]
    initial_condition = ${K_mass}
  []
  [f_Mg]
    initial_condition = ${Mg_mass}
  []
  [f_NO3]
    initial_condition = ${NO3_mass}
  []
  [f_Na]
    initial_condition = ${Na_mass}
  []
  [f_O2]
    initial_condition = ${O2_mass}
  []
  [f_SO4]
    initial_condition = ${SO4_mass}
  []
  [f_SiO2]
    initial_condition = ${SiO2_mass}
  []
  [f_HCO3]
    initial_condition = ${HCO3_mass}
  []
  [porepressure]
    initial_condition = ${porepressure}
  []
[]

[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = ${inlet_pressure}
    boundary = inlet
  []
  [constant_outer_porepressure]
    type = DirichletBC
    variable = porepressure
    value = ${outlet_pressure}
    boundary = outlet
  []
  [f_Al_in]
    type = DirichletBC
    variable = f_Al
    boundary = inlet
    value = ${Al_mass}
  []
  [f_Ca_in]
    type = DirichletBC
    variable = f_Ca
    boundary = inlet
    value = ${Ca_mass}
  []
  [f_Cl_in]
    type = DirichletBC
    variable = f_Cl
    boundary = inlet
    value = ${Cl_mass}
  []
  [f_F_in]
    type = DirichletBC
    variable = f_F
    boundary = inlet
    value = ${F_mass}
  []
  [f_Fe_in]
    type = DirichletBC
    variable = f_Fe
    boundary = inlet
    value = ${Fe_mass}
  []
  [f_H_in]
    type = DirichletBC
    variable = f_H
    boundary = inlet
    value = ${H_mass}
  []
  [f_K_in]
    type = DirichletBC
    variable = f_K
    boundary = inlet
    value = ${K_mass}
  []
  [f_Mg_in]
    type = DirichletBC
    variable = f_Mg
    boundary = inlet
    value = ${Mg_mass}
  []
  [f_NO3_in]
    type = DirichletBC
    variable = f_NO3
    boundary = inlet
    value = ${NO3_mass}
  []
  [f_Na_in]
    type = DirichletBC
    variable = f_Na
    boundary = inlet
    value = ${Na_mass}
  []
  [f_O2_in]
    type = DirichletBC
    variable = f_O2
    boundary = inlet
    value = ${O2_mass}
  []
  [f_SO4_in]
    type = DirichletBC
    variable = f_SO4
    boundary = inlet
    value = ${SO4_mass}
  []
  [f_SiO2_in]
    type = DirichletBC
    variable = f_SiO2
    boundary = inlet
    value = ${SiO2_mass}
  []
  [f_HCO3_in]
    type = DirichletBC
    variable = f_HCO3
    boundary = inlet
    value = ${HCO3_mass}
  []

  [f_Al]
    type = NeumannBC
    variable = f_Al
    boundary = outlet
  []
  [f_Ca]
    type = NeumannBC
    variable = f_Ca
    boundary = outlet
  []
  [f_Cl]
    type = NeumannBC
    variable = f_Cl
    boundary = outlet
  []
  [f_F]
    type = NeumannBC
    variable = f_F
    boundary = outlet
  []
  [f_Fe]
    type = NeumannBC
    variable = f_Fe
    boundary = outlet
  []
  [f_H]
    type = NeumannBC
    variable = f_H
    boundary = outlet
  []
  [f_K]
    type = NeumannBC
    variable = f_K
    boundary = outlet
  []
  [f_Mg]
    type = NeumannBC
    variable = f_Mg
    boundary = outlet
  []
  [f_NO3]
    type = NeumannBC
    variable = f_NO3
    boundary = outlet
  []
  [f_Na]
    type = NeumannBC
    variable = f_Na
    boundary = outlet
  []
  [f_O2]
    type = NeumannBC
    variable = f_O2
    boundary = outlet
  []
  [f_SO4]
    type = NeumannBC
    variable = f_SO4
    boundary = outlet
  []
  [f_SiO2]
    type = NeumannBC
    variable = f_SiO2
    boundary = outlet
  []
  [f_HCO3]
    type = NeumannBC
    variable = f_HCO3
    boundary = outlet
  []

[]


[Postprocessors]
  [mass_extracted_Al_in]
    type = SideAverageValue
    variable = f_Al
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Ca_in]
    type = SideAverageValue
    variable = f_Ca
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Cl_in]
    type = SideAverageValue
    variable = f_Cl
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_F_in]
    type = SideAverageValue
    variable = f_F
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Fe_in]
    type = SideAverageValue
    variable = f_Fe
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_H_in]
    type = SideAverageValue
    variable = f_H
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_HCO3_in]
    type = SideAverageValue
    variable = f_HCO3
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_K_in]
    type = SideAverageValue
    variable = f_K
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Mg_in]
    type = SideAverageValue
    variable = f_Mg
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_NO3_in]
    type = SideAverageValue
    variable = f_NO3
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Na_in]
    type = SideAverageValue
    variable = f_Na
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_O2_in]
    type = SideAverageValue
    variable = f_O2
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SO4_in]
    type = SideAverageValue
    variable = f_SO4
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SiO2_in]
    type = SideAverageValue
    variable = f_SiO2
    boundary = inlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Al_out]
    type = SideAverageValue
    variable = f_Al
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Ca_out]
    type = SideAverageValue
    variable = f_Ca
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Cl_out]
    type = SideAverageValue
    variable = f_Cl
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_F_out]
    type = SideAverageValue
    variable = f_F
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Fe_out]
    type = SideAverageValue
    variable = f_Fe
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_H_out]
    type = SideAverageValue
    variable = f_H
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_HCO3_out]
    type = SideAverageValue
    variable = f_HCO3
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_K_out]
    type = SideAverageValue
    variable = f_K
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Mg_out]
    type = SideAverageValue
    variable = f_Mg
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_NO3_out]
    type = SideAverageValue
    variable = f_NO3
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_Na_out]
    type = SideAverageValue
    variable = f_Na
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_O2_out]
    type = SideAverageValue
    variable = f_O2
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SO4_out]
    type = SideAverageValue
    variable = f_SO4
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted_SiO2_out]
    type = SideAverageValue
    variable = f_SiO2
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [darcy_velocity]
    type = SideAverageValue
    variable = darcy_vel_y
    boundary = outlet
    execute_on = 'initial timestep_end'
  []
  [mass_extracted]
    type = LinearCombinationPostprocessor
    pp_names = 'mass_extracted_Al_out mass_extracted_Ca_out mass_extracted_Cl_out mass_extracted_F_out mass_extracted_Fe_out mass_extracted_H_out mass_extracted_HCO3_out mass_extracted_K_out mass_extracted_Mg_out mass_extracted_NO3_out mass_extracted_Na_out mass_extracted_O2_out mass_extracted_SiO2_out mass_extracted_SO4_out'
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
    vel_y = darcy_vel_y
    #vel_z = darcy_vel_z
    boundary = outlet
    advected_variable = 1
    execute_on = 'initial timestep_end'
  []
  [delta_HCO3]
    type = DifferencePostprocessor
    value1 = mass_extracted_HCO3_out
    value2 = mass_extracted_HCO3_in
    execute_on = 'initial timestep_end'
  []

  [delta_Ca]
    type = DifferencePostprocessor
    value1 = mass_extracted_Ca_out
    value2 = mass_extracted_Ca_in
    execute_on = 'initial timestep_end'
  []

  [delta_Na]
    type = DifferencePostprocessor
    value1 = mass_extracted_Na_out
    value2 = mass_extracted_Na_in
    execute_on = 'initial timestep_end'
  []
  [delta_Al]
    type = DifferencePostprocessor
    value1 = mass_extracted_Al_out
    value2 = mass_extracted_Al_in
    execute_on = 'initial timestep_end'
  []
  [delta_Cl]
    type = DifferencePostprocessor
    value1 = mass_extracted_Cl_out
    value2 = mass_extracted_Cl_in
    execute_on = 'initial timestep_end'
  []
  [delta_F]
    type = DifferencePostprocessor
    value1 = mass_extracted_F_out
    value2 = mass_extracted_F_in
    execute_on = 'initial timestep_end'
  []
  [delta_Fe]
    type = DifferencePostprocessor
    value1 = mass_extracted_Fe_out
    value2 = mass_extracted_Fe_in
    execute_on = 'initial timestep_end'
  []
  [delta_H]
    type = DifferencePostprocessor
    value1 = mass_extracted_H_out
    value2 = mass_extracted_H_in
    execute_on = 'initial timestep_end'
  []
  [delta_K]
    type = DifferencePostprocessor
    value1 = mass_extracted_K_out
    value2 = mass_extracted_K_in
    execute_on = 'initial timestep_end'
  []
  [delta_Mg]
    type = DifferencePostprocessor
    value1 = mass_extracted_Mg_out
    value2 = mass_extracted_Mg_in
    execute_on = 'initial timestep_end'
  []
  [delta_NO3]
    type = DifferencePostprocessor
    value1 = mass_extracted_NO3_out
    value2 = mass_extracted_NO3_in
    execute_on = 'initial timestep_end'
  []
  [delta_O2]
    type = DifferencePostprocessor
    value1 = mass_extracted_O2_out
    value2 = mass_extracted_O2_in
    execute_on = 'initial timestep_end'
  []
  [delta_SO4]
    type = DifferencePostprocessor
    value1 = mass_extracted_SO4_out
    value2 = mass_extracted_SO4_in
    execute_on = 'initial timestep_end'
  []
  [delta_SiO2]
    type = DifferencePostprocessor
    value1 = mass_extracted_SiO2_out
    value2 = mass_extracted_SiO2_in
    execute_on = 'initial timestep_end'
  []

  # [delta_HCO3]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_HCO3_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []

  # [delta_Ca]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Ca_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []

  # [delta_Na]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Na_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_Al]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Al_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_Cl]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Cl_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_F]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_F_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_Fe]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Fe_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_H]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_H_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_K]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_K_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_Mg]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_Mg_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_NO3]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_NO3_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_O2]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_O2_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_SO4]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_SO4_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  # [delta_SiO2]
  #   type = ChangeOverTimePostprocessor
  #   postprocessor = mass_extracted_SiO2_out
  #   execute_on = 'initial timestep_end'
  #   change_with_respect_to_initial = true
  # []
  [avg_porosity]
    type = ElementAverageValue
    variable = chem_porosity
  [../]
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
  # multiply_by_density = true
  add_darcy_aux = true
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
  [chem_porosity]
    initial_condition = ${domain_porosity}
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityChemCoupled
    porosity = chem_porosity
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '${domain_permeability} 0 0   0 ${domain_permeability} 0   0 0 ${domain_permeability}'
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
  end_time = 259200 #126092.85
  nl_rel_tol = 1E-8
  [TimeSteppers]
    # [funcDT]
    #   type = FunctionDT
    #   function = 'if(t>500, 25, 10)'
    # []
    [const]
      type = ConstantDT
      dt = 1E4
    []
  []
[]


[Outputs]
  exodus = true
  csv = true
  [my_checkpoint]
    type = Checkpoint
    num_files = 4
    interval = 5
  []
[]

[MultiApps]
  [react]
    type = TransientMultiApp
    app_type = GeochemistryApp
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
    source_variable = 'massfrac_Al massfrac_Ca massfrac_Cl massfrac_F massfrac_Fe massfrac_H massfrac_HCO3 massfrac_K massfrac_Mg massfrac_Na massfrac_NO3 massfrac_O2 massfrac_SiO2 massfrac_SO4 porosity'
    variable = 'f_Al f_Ca f_Cl f_F f_Fe f_H f_HCO3 f_K f_Mg f_Na f_NO3 f_O2 f_SiO2 f_SO4 chem_porosity'
    from_multi_app = react
  []
[]
