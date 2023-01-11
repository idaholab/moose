#
#
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1 #10
    ny = 1
    xmin = 1.0
    xmax = 1.1
  []
  [move_nodes]
    type = MoveNodeGenerator
    input = MeshGenerator
    node_id = '0 2'
    new_position = '0.9 0.1 0 1.125 1.025 0'
  []
  [rotate]
    type = TransformGenerator
    input = move_nodes
    transform = rotate
    vector_value = '-20 0 0'
  []
[]

[Problem]
  coord_type = RZ
[]

[Functions]
  [pressure]
    type = ParsedFunction
    expression = 100*t
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        incremental = false
      []
    []
  []
[]

[BCs]
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [Pressure]
    [pressure]
      boundary = 'right'
      function = pressure
    []
  []
  # [pull_x]
  #   type = DirichletBC
  #   variable = disp_x
  #   boundary = left
  #   value = 1e-5
  #   preset = false
  # []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  []
#  [strain]
#    type = ComputeSmallStrain
#  []
  [stress]
    type = ComputeLinearElasticStress
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
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  petsc_options = '-snes_test_jacobian -snes_test_jacobian_view'
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 10
  end_time = 2.0
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
