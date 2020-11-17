[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 2
    zmin = 0
    zmax = 10
    nx = 6
    ny = 2
    nz = 2
    elem_type = HEX8
  []
  [corner]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '0 0 0'
    input = generated_mesh
  []
  [side]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '2 0 0'
    input = corner
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    use_finite_deform_jacobian = true
    volumetric_locking_correction = false
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz'
  []
[]

[Materials]
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e5 2.0e5 2.0e5 0.71428571e5 0.71428571e5 0.71428571e5 0.4 0.4 0.4 0.4 0.4 0.4' # Isotropic
  []
[]

[BCs]
  [fix_corner_x]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0
  []
  [fix_corner_y]
    type = DirichletBC
    variable = disp_y
    boundary = 101
    value = 0
  []
  [fix_side_y]
    type = DirichletBC
    variable = disp_y
    boundary = 102
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [move_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = 't'
  []
  [move_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = front
    function = 't*1.4'
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-12
  nl_max_its = 50

  l_tol = 1e-4
  l_max_its = 50

  dt = 0.4
  dtmin = 0.4

  num_steps = 1
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]
