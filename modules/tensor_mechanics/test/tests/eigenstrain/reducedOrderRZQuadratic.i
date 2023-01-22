#
# This test checks whether the ComputeReducedOrderEigenstrain is functioning properly.
#
# If instead of 'reduced_order_eigenstrain', 'thermal_eigenstrain' is given to
# eigenstrain_names in the Modules/TensorMechanics/Master/all block, the output will be
# quite different.
#
# Open the reducedOrderRZQuadratic_out_hydro_0001.csv file and plot the hydro variables as
# a function of x.
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
  second_order = true
[]

[Functions]
  [./tempLinear]
    type = ParsedFunction
    expression = '715-5*x'
  [../]
  [./tempQuadratic]
    type = ParsedFunction
    symbol_names = 'Tc Te'
    symbol_values = '701 700'
    expression = '(Te-Tc)/4.0*x*x+(Tc-Te)/2.0*x+Te+3.0*(Tc-Te)/4.0'
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
    initial_condition = 295.0
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
  [./thermal_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./thermal_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./thermal_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./reduced_constant]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./reduced_first]
    order = FIRST
    family = MONOMIAL
  [../]
  [./reduced_second]
    order = SECOND
    family = MONOMIAL
  [../]
  [./temp2]
    order = SECOND
    family = LAGRANGE
    initial_condition = 700
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        incremental = true
        temperature = temp2
        #eigenstrain_names = thermal_eigenstrain
        eigenstrain_names = reduced_order_eigenstrain
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
    execute_on = timestep_end
  [../]
  [./hydro_first_aux]
    type = RankTwoScalarAux
    variable = hydro_first
    rank_two_tensor = stress
    scalar_type = Hydrostatic
    execute_on = timestep_end
  [../]
  [./hydro_second_aux]
    type = RankTwoScalarAux
    variable = hydro_second
    rank_two_tensor = stress
    scalar_type = Hydrostatic
    execute_on = timestep_end
  [../]
  [./sxx_constant_aux]
    type = RankTwoAux
    variable = sxx_constant
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./sxx_first_aux]
    type = RankTwoAux
    variable = sxx_first
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./sxx_second_aux]
    type = RankTwoAux
    variable = sxx_second
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./szz_constant_aux]
    type = RankTwoAux
    variable = szz_constant
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./szz_first_aux]
    type = RankTwoAux
    variable = szz_first
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./szz_second_aux]
    type = RankTwoAux
    variable = szz_second
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./thermal_constant_aux]
    type = RankTwoAux
    variable = thermal_constant
    rank_two_tensor = thermal_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./thermal_first_aux]
    type = RankTwoAux
    variable = thermal_first
    rank_two_tensor = thermal_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./thermal_second_aux]
    type = RankTwoAux
    variable = thermal_second
    rank_two_tensor = thermal_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./reduced_constant_aux]
    type = RankTwoAux
    variable = reduced_constant
    rank_two_tensor = reduced_order_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./reduced_first_aux]
    type = RankTwoAux
    variable = reduced_first
    rank_two_tensor = reduced_order_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./reduced_second_aux]
    type = RankTwoAux
    variable = reduced_second
    rank_two_tensor = reduced_order_eigenstrain
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./temp2]
    type = FunctionAux
    variable = temp2
    function = tempQuadratic
    execute_on = timestep_begin
  [../]
[]

[BCs]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom #'bottom top'
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
    youngs_modulus = 1e8
    poissons_ratio = 0
  [../]
  [./fuel_thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    temperature = temp2
    stress_free_temperature = 295.0
    eigenstrain_name = 'thermal_eigenstrain'
  [../]
  [./reduced_order_eigenstrain]
    type = ComputeReducedOrderEigenstrain
    input_eigenstrain_names = 'thermal_eigenstrain'
    eigenstrain_name = 'reduced_order_eigenstrain'
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
  nl_rel_tol = 1e-8
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
    num_points = 50
    start_point = '1 0.07e-3 0'
    end_point = '3 0.07e-3 0'
    sort_by = x
    variable = 'temp2 disp_x disp_y hydro_constant hydro_first hydro_second sxx_constant sxx_first sxx_second szz_constant szz_first szz_second thermal_constant thermal_first thermal_second reduced_constant reduced_first reduced_second'
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]
