[Mesh]
  file = 1x1x1cube.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./top_pull]
    type = ParsedFunction
    expression = t*(0.0625)
  [../]
[]

[UserObjects]
  [dd_net]
    type = TorchScriptUserObject
    filename = "my_dd_net.pt"
    execute_on = "INITIAL"
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_yy plastic_strain_xx plastic_strain_yy plastic_strain_zz'
  [../]
[]

[BCs]
  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = top_pull
  [../]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./isotropic_plasticity]
    type = TorchPlasticityStressUpdate
    yield_stress = 50.0
    dislocation_density = dd_net
  [../]
  [./radial_return_stress]
    type = ComputeMultipleInelasticStress
    tangent_operator = elastic
    inelastic_models = 'isotropic_plasticity'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.00750
  dt = 0.00125
  dtmin = 0.0001
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
