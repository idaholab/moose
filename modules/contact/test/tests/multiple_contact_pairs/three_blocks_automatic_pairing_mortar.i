# Two tall blocks side by side on top of a wider bottom block, so that each block contacts the
# other two. The top blocks are compressed downward and confined laterally, closing both the
# horizontal and vertical interfaces. The meshes are nonconforming across every interface.
#
# Boundary centroids: base_top at (1, 1), left_bottom at (0.5, 1), right_bottom at (1.5, 1), and
# the two vertical faces left_right and right_left at (1, 2). With a pairing distance of 0.6, the
# pairs are (base_top, left_bottom), (base_top, right_bottom), and (right_left, left_right); every
# other centroid distance is at least 1. base_top is the primary surface of its pairs because it has
# the larger area. The two vertical faces have equal areas, so right_left, which has the larger
# boundary ID, is primary.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [bottom_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    ymax = 1
    nx = 5
    ny = 2
    subdomain_ids = 1
    boundary_name_prefix = base
    boundary_id_offset = 10
  []
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 1
    ymax = 3
    nx = 3
    ny = 4
    subdomain_ids = 2
    boundary_name_prefix = left
    boundary_id_offset = 20
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 1
    ymax = 3
    nx = 2
    ny = 3
    subdomain_ids = 3
    boundary_name_prefix = right
    boundary_id_offset = 30
  []
  [combine]
    type = CombinerGenerator
    inputs = 'bottom_block left_block right_block'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
        block = '1 2 3'
      []
    []
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2 3'
  []
[]

[Contact]
  [auto]
    automatic_pairing_boundaries = 'base_top left_bottom left_right right_bottom right_left'
    automatic_pairing_method = CENTROID
    automatic_pairing_distance = 0.6
    model = frictionless
    formulation = mortar
    c_normal = 1e6
    # The two vertical faces have equal lengths, so small relative motion of their ends leaves a
    # secondary end element slightly short of full primary coverage. Correct edge dropping keeps
    # the partially covered LM nodes active; otherwise all LM nodes of that element are zeroed
    correct_edge_dropping = true
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = base_bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = base_bottom
    value = 0
  []
  [confine_left]
    type = DirichletBC
    variable = disp_x
    boundary = left_left
    value = 0
  []
  [confine_right]
    type = DirichletBC
    variable = disp_x
    boundary = right_right
    value = 0
  []
  [push_down]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left_top right_top'
    function = '-1e-3 * t'
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 1
  end_time = 3
  nl_abs_tol = 1e-10
[]

[Outputs]
  [out]
    type = Exodus
    # The generated multi-pair subdomain and variable names exceed the default 32 characters and
    # would otherwise be truncated to identical names
    max_output_name_length = 80
  []
[]
