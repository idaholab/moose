[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = contact.e
  []
[]

[Variables]
  [temperature]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[Functions]
  [source]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 1 0'
  []
[]

[Modules/TensorMechanics/Master]
  [top_square]
    block = 1
    temperature = temperature
    add_variables = false
    strain = FINITE
  []
  [bottom_square]
    block = 2
    temperature = temperature
    add_variables = false
    strain = FINITE
    eigenstrain_names = thermal_eigenstrain
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
    block = '1 2'
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    value = 1500
    function = source
    block = 2
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = '1 2'
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 200
    thermal_expansion_coeff = 1.0e-4
    temperature = temperature
    eigenstrain_name = thermal_eigenstrain
    block = '2'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
[]

[Contact]
  [mechanical]
     model = frictionless
     formulation = mortar
     primary = 1
     secondary = 7
     c_normal = 1e+6
  []
[]

[BCs]
  [leftright_temp]
    type = DirichletBC
    variable = temperature
    boundary = '4 2'
    value = 200
  []
  [leftright_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = '4 2'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0
  []

  [bottom_disp_y_upper]
    type = DirichletBC
    variable = disp_y
    boundary = '5 6 8'
    value = 0
  []
  [bottom_disp_x_upper]
    type = DirichletBC
    variable = disp_x
    boundary = '5 6 8'
    value = 0
  []
  [temp_upper]
    type = DirichletBC
    variable = temperature
    boundary = 5
    value = 200
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none

  dt = 0.1
  dtmin = 0.01
  end_time = 1.0

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  l_max_its = 100
  nl_max_its = 25
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [peak_temp]
    type = NodalExtremeValue
    variable = temperature
  []
[]
