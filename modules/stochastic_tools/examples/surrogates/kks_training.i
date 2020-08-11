#length units: um
#2 phase, 2 component ideal solution model

[Mesh]
  type = GeneratedMesh
  dim  = 1
  nx   = 20
  xmax = 1000
  ny   = 1
  ymax = 1000
  uniform_refine = 5

[]

[GlobalParams]
  profile=TANH
[]

[Variables]
  #COMPONENT #1
  [./c_Ni]
  [../]
  [./c_Ni_metal]
  [../]
  [./c_Ni_melt]
  [../]
  [./w_Ni]
  [../]

  #COMPONENT #2
  [./c_Cr]
  [../]
  [./c_Cr_metal]
  [../]
  [./c_Cr_melt]
  [../]
  [./w_Cr]
  [../]
  [./eta]
  [../]
[]

[AuxVariables]


[]

[Kernels]
  #COMPONENT #1
  # =====================================enforce c = (1-h_metal(eta))*c_Ni_melt + h_metal(eta)*c_Ni_metal
  [./PhaseConc]
    type = KKSPhaseConcentration
    ca       = c_Ni_melt
    variable = c_Ni_metal
    c        = c_Ni
    eta      = eta
    h_name = h_metal
  [../]
  # ===========================enforce pointwise equality of chemical potentials
  [./ChemPot_Ni]
    type = KKSPhaseChemicalPotential
    variable = c_Ni_melt
    cb       = c_Ni_metal
    fa_name  = f_melt
    fb_name  = f_metal
    args_a = 'eta c_Cr c_Cr_melt'
    args_b = 'eta c_Cr c_Cr_metal'
  [../]
  # ======================================================Cahn-Hilliard Equation
  [./CHBulk]
    type = KKSSplitCHCRes
    variable = c_Ni
    ca       = c_Ni_melt
    fa_name  = f_melt
    w        = w_Ni
    args_a = 'eta c_Cr c_Cr_metal c_Cr_melt'
  [../]
  [./dcdt]
    type = CoupledTimeDerivative
    variable = w_Ni
    v = c_Ni
  [../]
  [./ckernel]
    type = SplitCHWRes
    mob_name = M_Ni
    variable = w_Ni
    args = 'c_Ni_metal c_Ni_melt eta'
  [../]

  #COMPONENT #2
  # =====================================enforce c = (1-h_metal(eta))*c_Ni_melt + h_metal(eta)*c_Ni_metal
  [./PhaseConc_Cr]
    type = KKSPhaseConcentration
    ca       = c_Cr_melt
    variable = c_Cr_metal
    c        = c_Cr
    eta      = eta
    h_name = h_metal
  [../]
  # ===========================enforce pointwise equality of chemical potentials
  [./ChemPot_Cr]
    type = KKSPhaseChemicalPotential
    variable = c_Cr_melt
    cb       = c_Cr_metal
    fa_name  = f_melt
    fb_name  = f_metal
    args_a = 'eta c_Ni c_Ni_melt c_Cr'
    args_b = 'eta c_Ni c_Ni_metal c_Cr'
  [../]
  # ======================================================Cahn-Hilliard Equation
  [./CHBulk_Cr]
    type = KKSSplitCHCRes
    variable = c_Cr
    ca       = c_Cr_melt
    fa_name  = f_melt
    w        = w_Cr
    args_a = 'eta c_Ni c_Ni_metal c_Ni_melt'
  [../]
  [./dcdt_Cr]
    type = CoupledTimeDerivative
    variable = w_Cr
    v = c_Cr
  [../]
  [./ckernel_Cr]
    type = SplitCHWRes
    mob_name = M_Cr
    variable = w_Cr
    args = 'c_Cr_metal c_Cr_melt eta'
  [../]


  # # =========================================================Allen-Cahn Equation
  [./ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name  = f_melt
    fb_name  = f_metal
    w = 6e7
    g_name = g
    h_name = h_metal
    mob_name = L
    args = 'c_Ni_melt c_Ni_metal c_Cr_metal c_Cr_melt c_Cr c_Ni'
  [../]
  [./ACBulkC_Ni]
    type = KKSACBulkC
    variable = eta
    ca       = c_Ni_melt
    cb       = c_Ni_metal
    fa_name  = f_melt
    # fb_name  = fd
    h_name = h_metal
    mob_name = L
    args = 'c_Ni_melt c_Ni_metal c_Cr_metal c_Cr_melt'
  [../]
  [./ACBulkC_Cr]
    type = KKSACBulkC
    variable = eta
    ca       = c_Cr_melt
    cb       = c_Cr_metal
    fa_name  = f_melt
    # fb_name  = fd
    h_name = h_metal
    mob_name = L
    args = 'c_Cr_melt c_Cr_metal c_Ni_metal c_Ni_melt'
  [../]

  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
    mob_name = L
    # args = 'c_Ni_melt c_Ni_metal c_Cr_metal c_Cr_melt'
  [../]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[ICs]
  [./eta_metal_inital]
    type = SmoothCircleIC
    variable = 'eta'
    int_width = 10
    x1 = 0
    y1 = 0
    radius = 100
    invalue = '1'
    outvalue = '0.0'
  [../]

#Ni INITIAL CONDITIONS
  [./c_global_inital]
      type = SmoothCircleIC
      variable = 'c_Ni'
      int_width = 10
      x1 = 0
      y1 = 0
      radius = 100
      invalue = '0.8'
      outvalue = 0.02
    [../]
  [./c_Ni_metal_inital]
    type = SmoothCircleIC
    variable = 'c_Ni_metal'
    int_width = 20
    x1 = 0
    y1 = 0
    radius = 100
    invalue = '0.8'
    outvalue = '0.99'
  [../]

  [./c_Ni_melt_inital]
    type = SmoothCircleIC
    variable = 'c_Ni_melt'
    int_width = 20
    x1 = 0
    y1 = 0
    radius = 100
    invalue = 0.02
    outvalue = 0.02
  [../]

#Cr INITIAL CONDITIONS
[./c_Cr_global_inital]
    type = SmoothCircleIC
    variable = 'c_Cr'
    int_width = 20
    x1 = 0
    y1 = 0
    radius = 100
    invalue = '0.19'
    outvalue = 0.003
  [../]
[./c_Cr_metal_inital]
  type = SmoothCircleIC
  variable = 'c_Cr_metal'
  int_width = 20
  x1 = 0
  y1 = 0
  radius = 100
  invalue = '0.19'
  outvalue = '8e-4'
[../]
[./c_Cr_melt_inital]
  type = SmoothCircleIC
  variable = 'c_Cr_melt'
  int_width = 20
  x1 = 0
  y1 = 0
  radius = 100
  invalue = 0.042
  outvalue = 0.003
[../]


[]

[Materials]
    [./constants]
      type = GenericConstantMaterial
      prop_names =  'gamma    L       interface_energy_sigma    interface_thickness_l   Va' #Va '
      prop_values = '1.5      1e2     1e8                          10                  1.09412844417051e-9'#1.09412844417051e-11
    [../]
    [./energy_constants]
      type = GenericConstantMaterial
      prop_names =  'kB            T     n  E0_Ni_metal  E0_Ni_melt    E0_N_metal   E0_N_melt  E0_Cr_metal  E0_Cr_melt k_metal k_melt'
      prop_values = '8.6173324e-5  1000  2  -0.4643      0.4817        -0.5441      -2.9913     -0.32       -0.88      3       3'
    [../]
    #PARAMETERS
    [./kappa] #assume that three interfaces having the same interfacial energy and thickness
      type = ParsedMaterial
      f_name = kappa
      material_property_names = 'interface_energy_sigma interface_thickness_l Va'
      function = '3*interface_energy_sigma*interface_thickness_l/4'
    [../]
    [./m]
      type = ParsedMaterial
      f_name = w
      material_property_names = 'interface_energy_sigma interface_thickness_l Va'
      function = '6*interface_energy_sigma/interface_thickness_l'
    [../]

        #SWITCHING FUNCTIONS
    [./h_metal]
      type = SwitchingFunctionMaterial
      function_name = 'h_metal'
      eta = 'eta'
      h_order = SIMPLE
      outputs = 'exodus'
      output_properties = 'h_metal'
    [../]

    #DOUBLE WELL eta
    [./g]
      type = DerivativeParsedMaterial
      f_name = 'g'
      args = 'eta'
      material_property_names = 'gamma'
      function = '(eta^4/4-eta^2/2)+((1-eta)^4/4-(1-eta)^2/2)+(gamma*eta^2*(1-eta)^2)+1/4'
    [../]

    #FREE ENERGY FUNCTIONS
    [./f_metal]
      type = DerivativeParsedMaterial
      f_name = f_metal
      args = 'eta w_Ni c_Ni c_Ni_metal w_Cr c_Cr_metal c_Cr'
      material_property_names = 'E0_Ni_metal E0_N_metal E0_Cr_metal k_metal kB T Va'
      function = '(E0_Ni_metal*c_Ni_metal + E0_Cr_metal*c_Cr + E0_N_metal*(1-c_Ni_metal -c_Cr_metal) +k_metal*kB*T*(c_Ni_metal*plog(c_Ni_metal,1e-3) + c_Cr_metal*plog(c_Cr_metal,1e-3) + (1-c_Ni_metal-c_Cr_metal)*plog(1-c_Ni_metal-c_Cr_metal,1e-3) ) )'
      outputs = exodus
    [../]
    [./f_melt]
      type = DerivativeParsedMaterial
      f_name = f_melt
      args = 'eta w_Ni c_Ni c_Ni_melt c_Cr c_Cr_melt w_Cr'
      material_property_names = 'E0_Ni_melt E0_N_melt E0_Cr_melt k_melt kB T Va'
      function = '(E0_Ni_melt*c_Ni_melt + E0_Cr_melt*c_Cr_melt + E0_N_melt*(1-c_Ni_melt -c_Cr_melt) +k_melt*kB*T*(c_Ni_melt*plog(c_Ni_melt,1e-3) + c_Cr_melt*plog(c_Cr_melt,1e-3) + (1-c_Ni_melt-c_Cr_melt)*plog(1-c_Ni_melt-c_Cr_melt,1e-3) ) )'
      outputs = exodus
    [../]

    [./susceptibility_Ni]
      type = DerivativeParsedMaterial
      f_name = 'chi_Ni'
      args = 'eta c_Ni c_Ni_metal c_Ni_melt'
      material_property_names = 'f_metal chi_inv_metal:=D[f_metal,c_Ni_metal,c_Ni_metal] f_melt chi_inv_melt:=D[f_melt,c_Ni_melt,c_Ni_melt] h_metal'
      function = 'h_metal/chi_inv_metal + (1-h_metal)/chi_inv_melt'
      outputs = exodus
    [../]

    [./mobility_Ni]
      type = DerivativeParsedMaterial
      f_name = M_Ni

      args = 'eta c_Ni c_Ni_metal c_Ni_melt'
      material_property_names = 'chi_Ni h_metal kB T Va'
      constant_names =        'D0_Ni_metal E0_Ni_metal D0_Ni_melt E0_Ni_melt'
      constant_expressions =  '0.92e8         2.88       0.92e16       2.88 '

      function = '(D0_Ni_metal*exp(-E0_Ni_metal/kB/T)*h_metal + D0_Ni_melt*exp(-E0_Ni_melt/kB/T)*(1-h_metal) )*chi_Ni'
      outputs = 'exodus'
      derivative_order = 2
    [../]

    [./susceptibility_Cr]
      type = DerivativeParsedMaterial
      f_name = 'chi_Cr'
      args = 'eta c_Cr c_Cr_metal c_Cr_melt'
      material_property_names = 'f_metal chi_inv_metal:=D[f_metal,c_Cr_metal,c_Cr_metal] f_melt chi_inv_melt:=D[f_melt,c_Cr_melt,c_Cr_melt] h_metal'
      function = 'h_metal/chi_inv_metal + (1-h_metal)/chi_inv_melt'
      outputs = exodus
    [../]

    [./mobility_Cr]
      type = DerivativeParsedMaterial
      f_name = M_Cr
      args = 'eta c_Cr c_Cr_metal c_Cr_melt'
      material_property_names = 'chi_Cr h_metal kB T Va'
      constant_names =        'D0_Ni_metal E0_Ni_metal D0_Ni_melt E0_Ni_melt'
      constant_expressions =  '0.92e8         2.88       0.92e16       2.88 '

      function = '(D0_Ni_metal*exp(-E0_Ni_metal/kB/T)*h_metal + D0_Ni_melt*exp(-E0_Ni_melt/kB/T)*(1-h_metal) )*chi_Cr'
      outputs = 'exodus'
      derivative_order = 2
    [../]


[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  scheme = bdf2
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  l_max_its = 30
  l_tol = 1e-10
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
  dtmin = 1e-7
  automatic_scaling = true
  compute_scaling_once = false
  scaling_group_variables = 'eta'
  dtmax = 0.05
  end_time = 1
  # num_steps = 1
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-2
    iteration_window = 2
    optimal_iterations = 9
    growth_factor = 1.1
    cutback_factor = 0.8
  [../]
[]

[Postprocessors]
  [./elapsed]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
[]

[Outputs]
  exodus = true
  file_base = 'kks_training/kks_training'
  perf_graph = true
  csv = true
[]
