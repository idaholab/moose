#
# This test checks whether the ComputeReducedOrderEigenstrain is functioning properly.
#
# If instead of 'fred', 'thermal_eigenstrain' is given to
# eigenstrain_names in the Modules/TensorMechanics/Master/all block, the output will be
# identical since the thermal strain is constant in the elements.
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
[]

[Functions]
  [./tempBC]
    type = ParsedFunction
    expression = '700+2*t*t'
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
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        incremental = true
        temperature = temp
        eigenstrain_names = 'fred' #'thermal_eigenstrain'
      [../]
    [../]
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
    type = FunctionDirichletBC
    variable = temp
    boundary = right
    function = tempBC
  [../]
  [./temp_left]
    type = FunctionDirichletBC
    variable = temp
    boundary = left
    function = tempBC
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
    thermal_expansion_coeff = 1e-6
    temperature = temp
    stress_free_temperature = 700.0
    eigenstrain_name = 'thermal_eigenstrain'
  [../]
  [./reduced_order_eigenstrain]
    type = ComputeReducedOrderEigenstrain
    input_eigenstrain_names = 'thermal_eigenstrain'
    eigenstrain_name = 'fred'
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

  dt = 1
  num_steps = 10
  nl_rel_tol = 1e-8
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
[]
