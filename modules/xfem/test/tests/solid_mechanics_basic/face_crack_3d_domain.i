[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

rad=0.1

[Mesh]
  #---- CUTTER MESH
  [cicle_outline]
    type = ParsedCurveGenerator
    x_formula = '${rad}*cos(t)'
    y_formula = '${rad}*sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '10'
    is_closed_loop = true
  []
  [circle_surface]
    type = XYDelaunayGenerator
    boundary = 'cicle_outline'
    desired_area = 0.001
    output_subdomain_id = 1
  []
  [circle_rotate]
    type = TransformGenerator
    input = circle_surface
    transform = ROTATE
    vector_value = '0 90 0'
  []
  [circle_move]
    type = TransformGenerator
    input = circle_rotate
    transform = TRANSLATE
    vector_value = '0 0 -0.01'
    save_with_name = mesh_cutter
  []
  #---- FEM MESH
  [FEM_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 14
    ny = 3
    nz = 8
    xmin = ${fparse -2*rad}
    xmax = ${fparse 2*rad}
    ymin = ${fparse -rad}
    ymax = ${fparse rad}
    zmin = 0
    zmax = ${fparse 2*rad}
    elem_type = HEX8
  []
  [pin]
    type = ExtraNodesetGenerator
    input = FEM_mesh
    new_boundary = 'pin'
    coord = '${fparse 2*rad} ${fparse -rad} ${fparse rad}'
    use_closest_node = true
  []
  final_generator = pin
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = -1e-3
  []
  [right_x]
    type = DirichletBC
    boundary = right
    variable = disp_x
    value = 0.0
  []
  [top_y]
    type = DirichletBC
    boundary = top
    variable = disp_y
    value = 0.0
  []
  [pin_z]
    type = DirichletBC
    boundary = pin
    variable = disp_z
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
    block = 0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []

  # controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 4
  max_xfem_update = 1
[]

[Outputs]
[]
