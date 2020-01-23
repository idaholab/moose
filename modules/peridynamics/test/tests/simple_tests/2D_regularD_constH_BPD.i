# Test for bond-based peridynamic formulation
# for regular grid from generated mesh with const bond constants

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
    formulation = BOND
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.33
  [../]

  [./force_density]
    type = ComputeSmallStrainConstantHorizonMaterialBPD
  [../]
[]

[Functions]
  [./disp_x_anal]
    type = PiecewiseLinear
    axis = x
    x = '0 1'
    y = '0 -0.00033'
  [../]
  [./disp_y_anal]
    type = PiecewiseLinear
    axis = y
    x = '0 1'
    y = '-0.001 0'
  [../]
[]

[Postprocessors]
  [./anal_disp_L2]
    type = NodalFunctionsL2NormPD
    functions = 'disp_x_anal disp_y_anal'
  [../]
  [./disp_diff_L2]
    type = NodalDisplacementDifferenceL2NormPD
    analytic_functions = 'disp_x_anal disp_y_anal'
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
  file_base = 2D_regularD_constH_BPD
  exodus = true
[]
