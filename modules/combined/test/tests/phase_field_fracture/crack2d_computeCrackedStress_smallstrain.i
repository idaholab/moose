#This input uses PhaseField-Nonconserved Action to add phase field fracture bulk rate kernels
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 20
    ymax = 0.5
  []
  [./noncrack]
    type = BoundingBoxNodeSetGenerator
    new_boundary = noncrack
    bottom_left = '0.5 0 0'
    top_right = '1 0 0'
    input = gen
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./strain_yy]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Physics]
  [./SolidMechanics]
    [./QuasiStatic]
      [./All]
        add_variables = true
        strain = SMALL
        planar_formulation = PLANE_STRAIN
        additional_generate_output = 'stress_yy'
        strain_base_name = uncracked
      [../]
    [../]
  [../]
[]
[Modules]
  [./PhaseField]
    [./Nonconserved]
      [./c]
        free_energy = E_el
        kappa = kappa_op
        mobility = L
      [../]
    [../]
  [../]
[]

[Kernels]
  [./solid_x]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_x
    component = 0
    c = c
  [../]
  [./solid_y]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_y
    component = 1
    c = c
  [../]
  [./off_disp]
    type = AllenCahnElasticEnergyOffDiag
    variable = c
    displacements = 'disp_x disp_y'
    mob_name = L
  [../]
[]

[AuxKernels]
  [./strain_yy]
    type = RankTwoAux
    variable = strain_yy
    rank_two_tensor = uncracked_mechanical_strain
    index_i = 1
    index_j = 1
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = 't'
  [../]
  [./yfix]
    type = DirichletBC
    variable = disp_y
    boundary = noncrack
    value = 0
  [../]
  [./xfix]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l visco'
    prop_values = '1e-3 0.05 1e-6'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '127.0 70.8 70.8 127.0 70.8 127.0 73.55 73.55 73.55'
    fill_method = symmetric9
    base_name = uncracked
    euler_angle_1 = 30
    euler_angle_2 = 0
    euler_angle_3 = 0
  [../]
  [./elastic]
    type = ComputeLinearElasticStress
    base_name = uncracked
  [../]
  [./cracked_stress]
    type = ComputeCrackedStress
    c = c
    kdamage = 1e-6
    F_name = E_el
    use_current_history_variable = true
    uncracked_base_name = uncracked
  [../]
[]

[Postprocessors]
  [./av_stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./av_strain_yy]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solving_package'
  petsc_options_value = 'lu superlu_dist'

  nl_rel_tol = 1e-8
  l_tol = 1e-4
  l_max_its = 100
  nl_max_its = 10

  dt = 5e-5
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
