[Mesh]
  file = nodal_area_Hex27.e
[]

[GlobalParams]
  order = SECOND
  displacements = 'displ_x displ_y displ_z'
[]

[Functions]
  [./disp]
    type = PiecewiseLinear
    x = '0     1'
    y = '0  20e-6'
  [../]
[]

[Variables]
  [./displ_x]
  [../]
  [./displ_y]
  [../]
  [./displ_z]
  [../]
[]

[AuxVariables]
  [./react_x]
  [../]
  [./react_y]
  [../]
  [./react_z]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    incremental = true
    save_in = 'react_x react_y react_z'
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx'
  [../]
[]

[BCs]
  [./move_right]
    type = FunctionDirichletBC
    boundary = '1'
    variable = displ_x
    function = disp
  [../]

  [./fixed_x]
    type = DirichletBC
    boundary = '3 4'
    variable = displ_x
    value = 0
  [../]

  [./fixed_y]
    type = DirichletBC
    boundary = 10
    variable = displ_y
    value = 0
  [../]

  [./fixed_z]
    type = DirichletBC
    boundary = 11
    variable = displ_z
    value = 0
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 3
    secondary = 2
    penalty = 1e8
    tangential_tolerance = 1e-4
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  #petsc_options_iname = '-snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart -pc_type'
  #petsc_options_value = 'ls         basic    basic                    201                lu'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-6

  l_tol = 1e-4
  l_max_its = 40

  start_time = 0.0
  dt = 1.0
  end_time = 1.0
  num_steps = 100

  [./Quadrature]
    order = THIRD
  [../]
[]

[Postprocessors]
  [./react_x]
    type = NodalSum
    variable = react_x
    boundary = 1
  [../]
  [./total_area]
    type = NodalSum
    variable = nodal_area
    boundary = 2
  [../]
[]

[Outputs]
  exodus = true
[]
