[Mesh]
  allow_renumbering = false
  [truss1]
    type = ElementGenerator
    nodal_positions = '0 0 -1
                       0 0 0'
    element_connectivity = '0 1'
    elem_type = EDGE2
    subdomain_id = 1
    subdomain_name = trusses
  []
  [truss2]
    type = ElementGenerator
    input = truss1
    nodal_positions = '1 0 -1
                       1 0 0'
    element_connectivity = '0 1'
    elem_type = EDGE2
    subdomain_id = 1
    subdomain_name = trusses
  []
  [truss3]
    type = ElementGenerator
    input = truss2
    nodal_positions = '1 1 -1
                       1 1 0'
    element_connectivity = '0 1'
    elem_type = EDGE2
    subdomain_id = 1
    subdomain_name = trusses
  []
  [truss4]
    type = ElementGenerator
    input = truss3
    nodal_positions = '0 1 -1
                       0 1 0'
    element_connectivity = '0 1'
    elem_type = EDGE2
    subdomain_id = 1
    subdomain_name = trusses
  []
  [stub]
    type = ElementGenerator
    input = truss4
    nodal_positions = '0.5 0.5 0
                       0.5 0.5 1'
    element_connectivity = '0 1'
    elem_type = EDGE2
    subdomain_id = 2
    subdomain_name = stub
  []
  [truss_bottoms]
    type = ExtraNodesetGenerator
    input = stub
    new_boundary = 'truss_bottoms'
    coord = '0 0 -1; 1 0 -1; 1 1 -1; 0 1 -1'
  []
  [corner1]
    type = ExtraNodesetGenerator
    input = truss_bottoms
    new_boundary = 'corner1'
    coord = '0 0 0'
  []
  [corner2]
    type = ExtraNodesetGenerator
    input = corner1
    new_boundary = 'corner2'
    coord = '1 0 0'
  []
  [corner3]
    type = ExtraNodesetGenerator
    input = corner2
    new_boundary = 'corner3'
    coord = '1 1 0'
  []
  [corner4]
    type = ExtraNodesetGenerator
    input = corner3
    new_boundary = 'corner4'
    coord = '0 1 0'
  []
  [ref_node]
    type = ExtraNodesetGenerator
    input = corner4
    new_boundary = 'ref_node'
    coord = '0.5 0.5 0'
  []
  [stub_top]
    type = ExtraNodesetGenerator
    input = ref_node
    new_boundary = 'stub_top'
    coord = '0.5 0.5 1'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
    block = stub
  []
  [rot_y]
    block = stub
  []
  [rot_z]
    block = stub
  []
[]

[AuxVariables]
  [truss_area]
    order = CONSTANT
    family = MONOMIAL
    block = trusses
  []
  [react_x]
  []
  [react_y]
  []
  [react_z]
  []
[]

[AuxKernels]
  [truss_area]
    type = ConstantAux
    variable = truss_area
    block = trusses
    value = 0.01
    execute_on = 'initial timestep_begin'
  []
[]

[Kernels]
  [truss_x]
    type = StressDivergenceTensorsTruss
    block = trusses
    component = 0
    variable = disp_x
    area = truss_area
    save_in = react_x
  []
  [truss_y]
    type = StressDivergenceTensorsTruss
    block = trusses
    component = 1
    variable = disp_y
    area = truss_area
    save_in = react_y
  []
  [truss_z]
    type = StressDivergenceTensorsTruss
    block = trusses
    component = 2
    variable = disp_z
    area = truss_area
    save_in = react_z
  []
  [stub_disp_x]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  []
  [stub_disp_y]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  []
  [stub_disp_z]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  []
  [stub_rot_x]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  []
  [stub_rot_y]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  []
  [stub_rot_z]
    type = StressDivergenceBeam
    block = stub
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  []
[]

[NodalKernels]
  [applied_force]
    type = UserForcingFunctorNodalKernel
    variable = disp_z
    boundary = stub_top
    functor = 1
  []
[]

[Constraints]
  [rbe3]
    type = RBE3Constraint
    reference_boundary = 'ref_node'
    independent_boundaries = 'corner1 corner2 corner3 corner4'
    weights = '1 1 1 1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
[]

[BCs]
  [fix_bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = truss_bottoms
    value = 0.0
  []
  [fix_bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = truss_bottoms
    value = 0.0
  []
  [fix_bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = truss_bottoms
    value = 0.0
  []
  [fix_corner_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'corner1 corner2 corner3 corner4'
    value = 0.0
  []
  [fix_corner_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'corner1 corner2 corner3 corner4'
    value = 0.0
  []
[]

[Materials]
  [truss_elasticity]
    type = LinearElasticTruss
    block = trusses
    youngs_modulus = 1e6
  []
  [stub_elasticity]
    type = ComputeElasticityBeam
    block = stub
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    shear_coefficient = 1.0
  []
  [stub_strain]
    type = ComputeIncrementalBeamStrain
    block = stub
    rotations = 'rot_x rot_y rot_z'
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    y_orientation = '0 1 0'
  []
  [stub_stress]
    type = ComputeBeamResultants
    block = stub
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
  line_search = none
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 1
  end_time = 2
[]

[Postprocessors]
  [force_corner1]
    type = NodalVariableValue
    variable = react_z
    nodeid = 0
    scale_factor = -1
  []
  [force_corner2]
    type = NodalVariableValue
    variable = react_z
    nodeid = 2
    scale_factor = -1
  []
  [force_corner3]
    type = NodalVariableValue
    variable = react_z
    nodeid = 4
    scale_factor = -1
  []
  [force_corner4]
    type = NodalVariableValue
    variable = react_z
    nodeid = 6
    scale_factor = -1
  []
[]

[Outputs]
  csv = true
[]
