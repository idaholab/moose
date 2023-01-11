#
#
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
  []
  [move_nodes]
    type = MoveNodeGenerator
    input = MeshGenerator
    node_id = 6
    new_position = '9.9 1.1 1'
  []
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
  [disp_z]
  []
[]

[Kernels]
  [TensorMechanics]
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  []
  [no_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  []
  [Pressure]
    [top]
      boundary = 'top front right'
      function = pressure
    []
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  []
  [strain]
    type = ComputeSmallStrain
  []
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
