[Mesh]
  allow_renumbering = false
  [solid]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 3
    nz = 3
    xmin = 0.0
    xmax = 5.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 1.0
    elem_type = HEX8
    subdomain_ids = '1'
    subdomain_name = 'solid'
  []
  [solid_tie_face]
    type = RenameBoundaryGenerator
    input = solid
    old_boundary = 'right'
    new_boundary = 'tie_face'
  []
  [stub_line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = 5.0
    xmax = 6.0
    elem_type = EDGE2
    subdomain_ids = '2'
    subdomain_name = 'stub'
  []
  [stub_moved]
    type = TransformGenerator
    input = stub_line
    transform = TRANSLATE
    vector_value = '0.0 0.5 0.5'
  []
  [stub_nodesets]
    type = RenameBoundaryGenerator
    input = stub_moved
    old_boundary = 'left right'
    new_boundary = 'stub_first_node stub_far_node'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'solid_tie_face stub_nodesets'
    avoid_merging_boundaries = true
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
    block = 'stub'
  []
  [rot_y]
    block = 'stub'
  []
  [rot_z]
    block = 'stub'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  displacements = 'disp_x disp_y disp_z'
  [solid]
    block = 'solid'
    strain = SMALL
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Physics/SolidMechanics/LineElement/QuasiStatic]
  [stub]
    block = 'stub'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.01
    Ay = 0.0
    Az = 0.0
    Iy = 1.0e-4
    Iz = 1.0e-4
    y_orientation = '0.0 1.0 0.0'
  []
[]

[Materials]
  [solid_elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 'solid'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [solid_stress]
    type = ComputeLinearElasticStress
    block = 'solid'
  []
  [stub_elasticity]
    type = ComputeElasticityBeam
    block = 'stub'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
    shear_coefficient = 1.0
  []
  [stub_stress]
    type = ComputeBeamResultants
    block = 'stub'
  []
[]

[BCs]
  [stub_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'stub_first_node'
    value = 0.0
  []
  [stub_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'stub_first_node'
    value = 0.0
  []
  [stub_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'stub_first_node'
    value = 0.0
  []
  [stub_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'stub_first_node'
    value = 0.0
  []
  [stub_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'stub_first_node'
    value = 0.0
  []
  [stub_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = 'stub_first_node'
    value = 0.01
  []
[]

[Constraints]
  [rbe2]
    type = RBE2Constraint
    independent_boundary = 'stub_first_node'
    dependent_boundary = 'tie_face'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    dependent_dofs = translations
  []
[]

[Functions]
  [rigid_disp_x]
    type = ParsedFunction
    expression = '-theta_z * (y - 0.5)'
    symbol_names = 'theta_z'
    symbol_values = '0.01'
  []
  [rigid_disp_y]
    type = ParsedFunction
    expression = 'theta_z * (x - 5.0)'
    symbol_names = 'theta_z'
    symbol_values = '0.01'
  []
  [rigid_disp_z]
    type = ParsedFunction
    expression = '0.0'
  []
[]

[Postprocessors]
  [error_disp_x]
    type = NodalL2Error
    variable = disp_x
    function = rigid_disp_x
    block = 'solid'
  []
  [error_disp_y]
    type = NodalL2Error
    variable = disp_y
    function = rigid_disp_y
    block = 'solid'
  []
  [error_disp_z]
    type = NodalL2Error
    variable = disp_z
    function = rigid_disp_z
    block = 'solid'
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
  line_search = 'none'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  dt = 1.0
  num_steps = 2
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
