# This test uses the strain calculator ComputePlaneSmallStrain,
# which is generated through the use of the TensorMechanics MasterAction.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    planar_formulation = PLANE_STRAIN
    add_variables = true
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
  [../]
[]

[Functions]
  [./pull]
    type = ParsedFunction
    expression ='0.01 * t'
  [../]
[]

[BCs]
  [./rightx]
    type = DirichletBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]
  [./pull]
    type = FunctionDirichletBC
    boundary = 2
    variable = disp_y
    function = pull
  [../]
[]

[Materials]
  [./linear_stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    poissons_ratio = 0.3
    youngs_modulus = 1e10
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_max_its = 100
  l_tol = 1e-10
  nl_max_its = 15
  nl_rel_tol = 1e-12

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 5.0
[]

[Outputs]
  exodus = true
[]
