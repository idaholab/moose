[GlobalParams]
  displacements = 'disp_x disp_y'
  scalar_out_of_plane_strain = scalar_strain_zz
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

  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules]
  [./Peridynamics/Mechanics]
    [./Master]
      [./all]
        formulation = ORDINARY_STATE
      [../]
    [../]
    [./GeneralizedPlaneStrain]
      [./all]
        formulation = ORDINARY_STATE
        out_of_plane_stress_variable = stress_zz
        out_of_plane_pressure = pressure_function
        factor = 1e5
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = NodalRankTwoPD
    variable = stress_zz
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    rank_two_tensor = stress
    output_type = component
    index_i = 2
    index_j = 2
  [../]
[]

[Postprocessors]
  [./react_z]
    type = NodalVariableIntegralPD
    variable = stress_zz
  [../]
[]

[Functions]
  [./pressure_function]
    type = PiecewiseLinear
    x = '0  1'
    y = '0  1'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    boundary = 1003
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 1000
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]

  [./force_density]
    type = ComputeSmallStrainConstantHorizonMaterialOSPD
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

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = out_of_plane_pressure_OSPD
[]
