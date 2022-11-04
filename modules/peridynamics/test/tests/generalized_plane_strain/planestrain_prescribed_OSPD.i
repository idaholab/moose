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
[]

[AuxVariables]
  [./temp]
  [../]
  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]

  [./strain_zz]
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = ORDINARY_STATE
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]

  [./strain_zz]
    type = NodalRankTwoPD
    variable = strain_zz
    rank_two_tensor = total_strain
    output_type = component
    index_i = 2
    index_j = 2
  [../]
[]

[AuxScalarKernels]
  [./scalar_strain_zz]
    type = FunctionScalarAux
    variable = scalar_strain_zz
    function = scalar_strain_zz_func
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1 - x) * t'
  [../]
  [./scalar_strain_zz_func]
    type = PiecewiseLinear
    xy_data = '0 0
               1 7.901e-5
               2 1.103021e-2'
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    boundary = 1000
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
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
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
  end_time = 2.0
[]

[Outputs]
  exodus = true
  file_base = planestrain_prescribed_OSPD
[]
