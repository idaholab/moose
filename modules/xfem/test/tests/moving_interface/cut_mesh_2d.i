[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 11
    ny = 11
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
    elem_type = QUAD4
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = block1
  []
[]

[AuxVariables]
  [u]
  []
[]

[UserObjects]
  [cut_mesh]
    type = InterfaceMeshCut2DUserObject
    mesh_file = circle_surface.e
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

[Physics/SolidMechanics/QuasiStatic]
  displacements = 'disp_x disp_y'
  [all]
    strain = SMALL
    add_variables = true
    incremental = false
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    displacements = 'disp_x disp_y'
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

[BCs]
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
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-12

  start_time = 0.0
  dt = 2
  end_time = 2

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
[]
