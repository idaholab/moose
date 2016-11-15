[Mesh]
  type = FileMesh
  file = crack_mesh.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./resid_x]
  [../]
  [./resid_y]
  [../]
[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    value = t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    save_in = 'resid_x resid_y'
    generate_output = 'stress_xx stress_yy stress_zz'
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = '1 2'
    value = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]
  [./elastic_stress]
    type = ComputeElasticSmearedCrackingStress
    cracking_release = exponential
    cracking_stress = 1.0
    cracking_residual_stress = 0.1
  [../]
[]

[Postprocessors]
  [./resid_x]
    type = NodalSum
    variable = resid_x
    boundary = 2
  [../]
  [./resid_y]
    type = NodalSum
    variable = resid_y
    boundary = 2
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1.0e-5

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  nl_max_its = 15

  dtmax = 100.0
  dtmin = 1.0e-12
  end_time = 3.0e-3
  dt = 1.0e-5
  num_steps = 5
[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]
