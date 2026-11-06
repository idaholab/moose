[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    xmin = 10
    xmax = 140
    ny = 4
    ymin = 0
    ymax = 100
    nz = 4
    zmin = 10
    zmax = 140
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
        incremental = false
        eigenstrain_names = 'reconst_eigenstrain'
        use_automatic_differentiation = false
        generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz vonmises_stress'
      []
    []
  []
[]

[UserObjects]
  [sol]
    type = SolutionUserObject
    mesh = 'simple_2d_out_0000_mesh.xda'
    es   = 'simple_2d_out_0000.xda'
    timestep = LATEST
    force_replicated_source_mesh = true
    system = aux0
    system_variables = 'eig_rr eig_yy eig_ry eig_tt'
  []
[]

[Functions]
  [f_xx]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 0
    component_j = 0
  []
  [f_yy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 1
    component_j = 1
  []
  [f_zz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 2
    component_j = 2
  []
  [f_xy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 0
    component_j = 1
  []
  [f_xz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 0
    component_j = 2
  []
  [f_yz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'eig_rr eig_yy eig_ry eig_tt'
    component_i = 1
    component_j = 2
  []
[]

[Materials]
  [reconst_eigenstrain]
    type = GenericFunctionRankTwoTensor
    tensor_name = reconst_eigenstrain
    tensor_functions = 'f_xx f_xy f_xz
                        f_xy f_yy f_yz
                        f_xz f_yz f_zz'
  []
  [elastic]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 183150
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
