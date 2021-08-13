[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 11
    ny = 11
    nz = 1
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 0.1
    elem_type = HEX8
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0.1'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0.1'
    input = block1
  []
[]

[UserObjects]
  [cut_mesh]
    type = InterfaceMeshCut3DUserObject
    mesh_file = cylinder_surface.e
    interface_velocity_function = vel_func
    heal_always = true
    block = 2
  []
[]

[Functions]
  [vel_func]
    type = ConstantFunction
    value = 0.011
  []
[]

[Modules/TensorMechanics/Master]
  displacements = 'disp_x disp_y disp_z'
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [ls]
  []
[]

[AuxKernels]
  [ls]
    type = MeshCutLevelSetAux
    mesh_cut_user_object = cut_mesh
    variable = ls
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = 1
  []
  [time_deriv]
    type = TimeDerivative
    variable = u
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [front_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [back_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [box1_x]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = left
  []
  [box1_y]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = left
  []
  [box1_z]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = left
  []
  [box2_x]
    type = FunctionDirichletBC
    variable = disp_x
    function = '0.01*t'
    boundary = right
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 20
  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

  start_time = 0.0
  dt = 2
  end_time = 2

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
[]
