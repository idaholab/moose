[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 8
    ny = 8
    xmin = -82.627
    xmax = 82.627
    ymin = -82.627
    ymax = 82.627
    dim = 2
  []
  [./extra_nodes_x]
    type = ExtraNodesetGenerator
    input = 'gen'
    new_boundary = 'no_x'
    coord = '0 82.627 0'
  [../]
  [./extra_nodes_y]
    type = ExtraNodesetGenerator
    input = 'extra_nodes_x'
    new_boundary = 'no_y'
    coord = '-82.627 0 0'
  [../]
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
[]

[AuxVariables]
  [./temp]
  [../]
[]

[Modules/TensorMechanics/Master]
  # FINITE strain when strain is large, i.e., visible movement.
  # SMALL strain when things are stressed, but may not move.
  [./fuel]
    add_variables = true
    strain = FINITE
    temperature = temp
    eigenstrain_names = 'thermal_eigenstrain'
    generate_output = 'vonmises_stress stress_xx stress_yy hydrostatic_stress max_principal_stress strain_xy elastic_strain_xx stress_xy'
    extra_vector_tags = 'ref'
    use_finite_deform_jacobian = true
    incremental = true
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'no_x'
    value = 0.0
    preset = true
  [../]
  [./no_y]
    type = DirichletBC
    preset = true
    variable = disp_y
    boundary = 'no_y'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3e10   # Pa
    poissons_ratio = 0.33    # unitless
  [../]
  [./thermal_strains]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 2e-6 # 1/K
    stress_free_temperature = 500 # K
    eigenstrain_name = 'thermal_eigenstrain'
  [../]
  [./stress_finite] # goes with FINITE strain formulation
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Postprocessors]
  [./avg_temp]
    type = ElementAverageValue
    variable = temp
    execute_on = 'initial timestep_end'
  [../]
  [./disp_x_max_element]
    type = ElementExtremeValue
    value_type = max
    variable = disp_x
    execute_on = 'initial timestep_end'
  [../]
  [./disp_y_max_element]
    type = ElementExtremeValue
    value_type = max
    variable = disp_y
    execute_on = 'initial timestep_end'
  [../]
  [./disp_x_max_nodal]
    type = NodalExtremeValue
    value_type = max
    variable = disp_x
    execute_on = 'initial timestep_end'
  [../]
  [./disp_y_max_nodal]
    type = NodalExtremeValue
    value_type = max
    variable = disp_y
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 300'
  line_search = 'none'

  l_tol = 1e-02
  nl_rel_tol = 5e-04
  nl_abs_tol = 1e-2

  l_max_its = 50
  nl_max_its = 25

  start_time = 0
  end_time = 40
  dt = 10
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  exodus = true
[]
