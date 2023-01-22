# This test uses the strain calculator ComputePlaneFiniteStrain,
# which is generated through the use of the TensorMechanics MasterAction.

[Mesh]
  type = GeneratedMesh
  nx = 2
  ny = 2
  dim = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    planar_formulation = PLANE_STRAIN
    add_variables = true
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
  [../]
[]

[Functions]
  [./pull]
    type = ParsedFunction
    expression ='0.005 * t'
  [../]
[]

[BCs]
  [./leftx]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./pull]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = pull
  [../]
[]

[Materials]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_max_its = 100
  l_tol = 1e-10
  nl_max_its = 10
  nl_rel_tol = 1e-12

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
