# Test for ordinary state-based peridynamic formulation
# for irregular grid from file mesh with varying bond constants
# partial Jacobian
# Jacobian from bond-based formulation is used for preconditioning

# Square plate with Dirichlet boundary conditions applied
# at the left, top and bottom edges

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = square.e
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
  [../]
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[BCs]
  [./left_dx]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./top_dy]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
  [./bottom_dy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = '-0.001 * t'
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
  file_base = 2D_irregularD_variableH_OSPD
  exodus = true
[]
