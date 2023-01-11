#
# This test checks whether the ComputeReducedOrderEigenstrain is functioning properly
# when using the automatic_eigenstrain_names within the TensorMechanicsAction.  These
# results should match the results found in the eigenstrain folder for reducedOrderRZLinear.i
#

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
  xmax = 3
  xmin = 1
  ymax = 1
  ymin = 0
  #second_order = true
[]

[Problem]
  solve = false
[]

[Functions]
  [./tempLinear]
    type = ParsedFunction
    expression = '715-5*x'
  [../]
  [./tempQuadratic]
    type = ParsedFunction
    expression = '2.5*x*x-15*x+722.5'
  [../]
  [./tempCubic]
    type = ParsedFunction
    expression = '-1.25*x*x*x+11.25*x*x-33.75*x+733.75'
  [../]
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 700
  [../]
[]

[AuxVariables]
  [./hydro_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydro_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./hydro_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./sxx_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sxx_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./sxx_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./szz_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./szz_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./szz_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./temp2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 700
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = SMALL
    incremental = true
    temperature = temp2
    automatic_eigenstrain_names = true
  [../]
[]

[Kernels]
  [./heat]
    type = Diffusion
    variable = temp
  [../]
[]

[AuxKernels]
  [./hydro_constant_aux]
    type = RankTwoScalarAux
    variable = hydro_constant
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  [../]
  [./hydro_first_aux]
    type = RankTwoScalarAux
    variable = hydro_first
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  [../]
  [./hydro_second_aux]
    type = RankTwoScalarAux
    variable = hydro_second
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  [../]
  [./sxx_constant_aux]
    type = RankTwoAux
    variable = sxx_constant
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./sxx_first_aux]
    type = RankTwoAux
    variable = sxx_first
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./sxx_second_aux]
    type = RankTwoAux
    variable = sxx_second
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./szz_constant_aux]
    type = RankTwoAux
    variable = szz_constant
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
  [./szz_first_aux]
    type = RankTwoAux
    variable = szz_first
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
  [./szz_second_aux]
    type = RankTwoAux
    variable = szz_second
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
  [./temp2]
    type = FunctionAux
    variable = temp2
    function = tempLinear
    execute_on = timestep_begin
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0.0
  [../]

  [./temp_right]
    type = DirichletBC
    variable = temp
    boundary = right
    value = 700
  [../]
  [./temp_left]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 710
  [../]
[]


[Materials]
  [./fuel_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0
  [../]
  [./fuel_thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1
    temperature = temp2
    stress_free_temperature = 700.0
    eigenstrain_name = 'thermal_eigenstrain'
  [../]
  [./reduced_order_eigenstrain]
    type = ComputeReducedOrderEigenstrain
    input_eigenstrain_names = 'thermal_eigenstrain'
    eigenstrain_name = 'reduced_eigenstrain'
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
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew '
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type'
  petsc_options_value = '70 hypre boomeramg'

  num_steps = 1
  nl_rel_tol = 1e-8 #1e-12
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[VectorPostprocessors]
  [./hydro]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    num_points = 100
    start_point = '1 0.07e-3 0'
    end_point = '3 0.07e-3 0'
    sort_by = x
    variable = 'hydro_constant hydro_first hydro_second temp2 disp_x disp_y'
  [../]
[]

[Outputs]
  exodus = true
[]
