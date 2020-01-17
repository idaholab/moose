# Test for ordinary state-based peridynamic formulation
# for regular grid from generated mesh with varying bond constants
# partial Jacobian
# Jacobian from bond-based formulation is used for preconditioning

# Square plate with Dirichlet boundary conditions applied
# at the left, top and bottom edges

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1003
    value = 0.0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1002
    value = 0.0
  [../]
  [./bottom_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1000
    function = '-0.001*t'
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = ORDINARY_STATE
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.0
  [../]

  [./force_density]
    type = ComputeSmallStrainVariableHorizonMaterialOSPD
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
  solve_type = PJFNK
  line_search = none
  start_time = 0
  end_time = 1
[]

[Outputs]
  file_base = 2D_regularD_variableH_OSPD
  exodus = true
[]
