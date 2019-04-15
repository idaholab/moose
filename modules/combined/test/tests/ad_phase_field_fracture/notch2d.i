[Mesh]
  type = FileMesh
  file = notch2d.e
[]

[MeshModifiers]
 [./bottom_left]
  type = AddExtraNodeset
  new_boundary = 'bottom_left'
  coord = '-12.7 -76.2'
 [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hist]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./bounds_dummy]
  order = FIRST
  family = LAGRANGE
[../]
[]

[AuxKernels]
  [./elastic_strain_yy]
    type = MaterialRankTwoTensorAux
    variable = elastic_strain_yy
    i = 1
    j = 1
    property = elastic_strain
  [../]
  [./plastic_strain_yy]
    type = MaterialRankTwoTensorAux
    variable = plastic_strain_yy
    i = 1
    j = 1
    property = plastic_strain
  [../]
  [./strain_yy]
    type = MaterialRankTwoTensorAux
    variable = strain_yy
    i = 1
    j = 1
    property = mechanical_strain
  [../]
  [./stress_yy]
    type = MaterialRankTwoTensorAux
    variable = stress_yy
    i = 1
    j = 1
    property = cauchy_stress
  [../]
  [./hist]
    type = MaterialRealAux
    variable = hist
    property = hist
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

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l'
    #prop_values = '8.44e-3 3.226e-4'
    prop_values = '10.5 0.4'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '68.8e3 0.33'
    fill_method = symmetric_isotropic_E_nu
  [../]
  [./strain]
    type = ADComputeGreenLagrangeStrain
  [../]
  [./elastic]
    type = ADComputeHyperElastoPlasticPFFractureStress
    yield_stress = 320
    linear_hardening_coefficient = 688
    beta_p = true
    beta_e = true
    W0 = 0
    c = c
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = 't'
  [../]
  [./yfix]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_left
    value = 0
  [../]
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./plastic_strain_yy]
    type = ElementAverageValue
    variable = plastic_strain_yy
  [../]
  [./elastic_strain_yy]
    type = ElementAverageValue
    variable = elastic_strain_yy
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  [../]
  [./hist]
    type = ElementAverageValue
    variable = hist
  [../]
[]

[Bounds]
  [./v_bounds]
    type = PFFractureBoundsAux
    variable = bounds_dummy
    bounded_variable = c
    upper = 1.0
    lower = 0.0
    lower_var = c
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = basic

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-7
  l_tol = 1e-4
  l_max_its = 50
  nl_max_its = 50

  dtmin = 1e-8
  dt = 1e-3
  dtmax = 1.0e-3
  end_time = 3

  [./TimeStepper]
  type = IterationAdaptiveDT
  dt = 1.0e-3
  optimal_iterations = 4
  iteration_window = 1
  cutback_factor = 0.5
  growth_factor = 1.2
[../]
[]

[Outputs]
  exodus = true
  csv = true
[]
