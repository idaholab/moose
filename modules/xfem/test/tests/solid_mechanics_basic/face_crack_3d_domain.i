[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

rad=0.1
offset = 0
spin = 0
tilt = 0
fname = 'tet_block'
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
  [circle_spin]
    type = TransformGenerator
    input = circle_surface
    transform = ROTATE
    vector_value = '0 0 ${spin}'
  []
  [circle_rotate]
    type = TransformGenerator
    input = circle_spin
    transform = ROTATE
    vector_value = '0 ${fparse 90-tilt} 0'
  []
  [circle_move]
    type = TransformGenerator
    input = circle_rotate
    transform = TRANSLATE
    vector_value = '${offset} 0 -0.01'
    save_with_name = mesh_cutter
  []

  #---- FEM MESH
  [FEM_mesh]
    type = FileMeshGenerator
    file = ${fname}.e
  []
  [FEM_mesh_move]
    type = TransformGenerator
    input = FEM_mesh
    transform = TRANSLATE
    vector_value = '0 -0.0001 0'
  []
  [pin]
    type = ExtraNodesetGenerator
    input = FEM_mesh_move
    new_boundary = 'pin'
    coord = '${fparse 2*rad} ${fparse -rad} ${fparse rad}'
    use_closest_node = true
  []
  [center]
    type = ExtraNodesetGenerator
    input = pin
    new_boundary = 'center'
    coord = '0 ${fparse rad} 0'
    use_closest_node = true
  []
  final_generator = center
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
    value = 0.0
  []
  [right_x]
    type = DirichletBC
    boundary = right
    variable = disp_x
    value = 0.0
  []
  # Ramp traction in z so KI varies along the crack front:
  # near z=0 (surface, inactive nodes): below k_low -> no growth
  # mid-front: transition zone -> moderate growth
  # deepest nodes (z~0.09): above k_high -> max growth
  [top_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = '20 + 500 * z'
  []
  [bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
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
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
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

  l_max_its = 100
  l_tol = 1e-2

  nl_max_its = 15
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1.0
  end_time = 4
  max_xfem_update = 1
[]

[Outputs]
[]
