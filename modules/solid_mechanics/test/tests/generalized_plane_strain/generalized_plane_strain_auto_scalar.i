[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [saved_x]
    order = FIRST
    family = LAGRANGE
  []
  [saved_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[Postprocessors]
  [react_z]
    type = MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [gps]
        planar_formulation = GENERALIZED_PLANE_STRAIN
        strain = SMALL
        scalar_out_of_plane_strain = scalar_strain_zz
        out_of_plane_pressure_function = traction_function
        pressure_factor = 1e5
        generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
        save_in = 'saved_x saved_y'
      []
    []
  []
[]

[Functions]
  [traction_function]
    type = PiecewiseLinear
    x = '0  2'
    y = '0  1'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    boundary = 3
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [traction_material]
    type = GenericFunctionMaterial
    prop_names = traction_material
    prop_values = traction_function
  []
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_max_its = 100
  l_tol = 1e-4

  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-11

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
