[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  group_variables = 'disp_x disp_y'
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [eng1]
    type = ExtraNodesetGenerator
    input = square
    new_boundary = 'lower_left'
    coord = '0 0'
  []
  [eng2]
    type = ExtraNodesetGenerator
    input = eng1
    new_boundary = 'lower_right'
    coord = '1 0'
  []
[]

[AuxVariables]
  [temp]
  []
  [ref_x]
  []
  [ref_y]
  []
[]

[AuxKernels]
  [tempfuncaux]
    type = FunctionAux
    variable = temp
    function = '(1-x)*t'
    use_displaced_mesh = false
  []
  [ref_x]
    type = TagVectorAux
    variable = 'ref_x'
    vector_tag = 'ref'
    v = 'disp_x'
    execute_on = timestep_end
  []
  [ref_y]
    type = TagVectorAux
    variable = 'ref_y'
    vector_tag = 'ref'
    execute_on = timestep_end
    v = 'disp_y'
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    boundary = 'lower_left'
    variable = disp_x
    value = 0.0
  []
  [fix_y]
    type = DirichletBC
    boundary = 'lower_left lower_right'
    variable = disp_y
    value = 0.0
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    add_variables = true
    temperature = temp
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
    planar_formulation = PLANE_STRAIN
    eigenstrain_names = eigenstrain
    absolute_value_vector_tags = 'ref'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [elastic_stress]
    type = ComputeLinearElasticStress
  []
  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 0.02
    temperature = temp
    stress_free_temperature = 0.5
    eigenstrain_name = eigenstrain
  []
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_max_its = 100
  l_tol = 1e-8

  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
