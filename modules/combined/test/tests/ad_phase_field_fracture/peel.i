[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = FileMesh
  file = 'mesh.msh'
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

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
  [../]
[]

[BCs]
  [./xdisp_top]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'top'
    function = '(x-1)*(cos(t)-1)+0.5*sin(t)'
  [../]
  [./ydisp_top]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'top'
    function = '-(x-1)*sin(t)+0.5*(cos(t)-1)'
  [../]
  [./xdisp_right]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'right_upper'
    function = '(y-0.5)*sin(t)'
  [../]
  [./ydisp_right]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'right_upper'
    function = '(y-0.5)*(cos(t)-1)'
  [../]
  [./xfix_bottom]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'bottom'
    function = '0'
  [../]
  [./yfix_bottom]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'bottom'
    function = '0'
  [../]
  [./xfix_right]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'right_lower'
    function = '0'
  [../]
  [./yfix_right]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'right_lower'
    function = '0'
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l'
    prop_values = '2.7 0.06'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2.1e5 0.3'
    fill_method = symmetric_isotropic_E_nu
  [../]
  [./strain]
    type = ADComputeGreenLagrangeStrain
  [../]
  [./elastic]
    type = ADComputeFiniteStrainElasticPFFractureStress
    c = c
    kdamage = 1.0e-6
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

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  end_time = 1.57
  dtmax = 1e-3
  dtmin = 1e-10
  nl_max_its = 50

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-5
    optimal_iterations = 5
    iteration_window = 1
    cutback_factor = 0.5
    growth_factor = 1.1
  [../]
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  interval = 1
[]
