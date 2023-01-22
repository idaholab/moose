[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    expression = '0.01 * t'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    decomposition_method = EigenSolution
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = tdisp
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomeramg

  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  nl_rel_tol = 1e-10

  dt = 0.05
  dtmin = 0.05
  nl_abs_step_tol = 1e-10

  num_steps = 10
[]

[Outputs]
  exodus = true
[]
