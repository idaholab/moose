[Mesh]
  type = FileMesh
  file = sent.e
  uniform_refine = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshModifiers]
  [./node1]
    type = AddExtraNodeset
    new_boundary = '101'
    coord = '0 0'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
  [../]
[]

[Kernels]
  [./stress_x]
    type = ADStressDivergenceTensors
    component = 0
    variable = disp_x
  [../]
  [./stress_y]
    type = ADStressDivergenceTensors
    component = 1
    variable = disp_y
  [../]
  [./ad_pf]
    type = ADPhaseFieldFracture
    l_name = l
    gc = gc_prop
    variable = c
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l'
    prop_values = '1e-3 0.04'
  [../]
  [./strain]
    type = ADComputeGreenLagrangeStrain
  [../]
  [./elastic]
    type = ADComputeFiniteStrainElasticPFFractureStress
    c = c
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '208 0.3'
    fill_method = symmetric_isotropic_E_nu
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 2
    function = 't'
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = '1 2'
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
    ksp_norm = default
  [../]
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  #line_search = default

  #line_search = none

  nl_rel_tol = 1e-8 #-7
  nl_abs_tol = 1e-10 #-6
  l_tol = 1e-4
  l_max_its = 20
  nl_max_its = 25

  dt = 1e-4 #-4
  # dtmin = 1.0e-5
  num_steps = 200

  dtmax = 1e-4 #-4
  #end_time = 0.00625
# [./TimeStepper]
#    type = IterationAdaptiveDT
#    dt = 1e-5
#    optimal_iterations = 10 #3
#    iteration_window = 0
#    growth_factor = 1.2
#    cutback_factor = 0.75
#  [../]

# picard_max_its = 1000
# picard_abs_tol = 1e-3
# picard_rel_tol = 1e-3

[]

[Outputs]
  exodus = true
  #print_perf_log = true
  csv = true
  print_linear_residuals = false
[]
