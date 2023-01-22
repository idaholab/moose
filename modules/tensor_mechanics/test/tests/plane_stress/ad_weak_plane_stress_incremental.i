[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
  out_of_plane_strain = strain_zz
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./strain_zz]
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./nl_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Postprocessors]
  [./react_z]
    type = ADMaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
  [./min_strain_zz]
    type = NodalExtremeValue
    variable = strain_zz
    value_type = min
  [../]
  [./max_strain_zz]
    type = NodalExtremeValue
    variable = strain_zz
    value_type = max
  [../]
[]

[Modules/TensorMechanics/Master]
  [./plane_stress]
    planar_formulation = WEAK_PLANE_STRESS
    strain = SMALL
    incremental = true
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy'
    eigenstrain_names = eigenstrain
    use_automatic_differentiation = true
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
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    variable = nl_strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x='0     1   100'
    y='0  0.00  0.00'
  [../]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1 - x) * t'
  [../]
[]

[BCs]
  [./bottomx]
    type = DirichletBC
    boundary = 0
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ADComputeStrainIncrementBasedStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-12

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  file_base = 'weak_plane_stress_incremental_out'
  exodus = true
[]
