[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [ring]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[BCs]
  [fix_x1]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0
  []
  [fix_x2]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = 0.1*sin(t)
  []
  [fix_y]
    type = DirichletBC
    boundary = 'left right'
    variable = disp_y
    value = 0
  []
  [fix_z]
    type = DirichletBC
    boundary = 'left right'
    variable = disp_z
    value = 0
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'vonmises_stress effective_alt_total_strain'
  []
[]

[Materials]
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [elastic]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 0.3
    shear_modulus = 100
  []
  [alt_strain]
    type = ComputeFiniteStrain
    base_name = alt
  []
[]

[Executioner]
  type = Transient
  num_steps = 12
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  perf_graph = true
[]
