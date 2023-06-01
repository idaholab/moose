[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
  elem_type = EDGE3
[]

[ICs]
  active = 'constant_elem constant_nodal'
  [constant_elem]
    type = ConstantIC
    variable = base_elem
    value = 4
  []
  [constant_nodal]
    type = ConstantIC
    variable = base_nodal
    value = 3.5
  []
  [linear_elem]
    type = FunctionIC
    variable = base_elem
    function = 2+2*x
  []
  [linear_nodal]
    type = FunctionIC
    variable = base_nodal
    function = 3+3*x
  []
  [quadratic_elem]
    type = FunctionIC
    variable = base_elem
    function = 2+2*x*x
  []
  [quadratic_nodal]
    type = FunctionIC
    variable = base_nodal
    function = 3+3*x*x
  []
[]

[AuxVariables]
  # Families:
  # LAGRANGE, MONOMIAL, HERMITE, SCALAR, HIERARCHIC, CLOUGH, XYZ, SZABAB, BERNSTEIN,
  # L2_LAGRANGE, L2_HIERARCHIC, RATIONAL_BERNSTEIN, SIDE_HIERARCHIC
  # Notes:
  # - 'elemental': MONOMIAL, XYZ, L2_LAGRANGE, L2_HIERARCHIC
  # - 'nodal': LAGRANGE, HERMITE, HIERARCHIC, CLOUGH, SZABAB, BERNSTEIN, RATIONAL_BERNSTEIN
  # - Hermite, Hierachic, Clough, Bernstein, rational Bernstein cannot be created on 1D EDGE3 elements
  # - Hermite, Hierarchic, Clough, Bernstein, rational Bernstein, Szabab, L2_lagrange, L2_hierarchic
  #   cannot be created with a constant order
  [base_elem]
    family = MONOMIAL
    order = CONSTANT
  []
  [base_nodal]
  []
  [test_elem_lagrange]
  []
  [test_elem_lagrange_high]
    order = SECOND
  []
  [test_elem_mono]
    family = MONOMIAL
    order = CONSTANT
  []
  [test_elem_mono_high]
    family = MONOMIAL
    order = SECOND
  []
  [test_elem_fv]
    type = MooseVariableFVReal
  []
  [test_elem_xyz]
    family = XYZ
    order = CONSTANT
  []
  [test_elem_xyz_high]
    family = XYZ
    order = SECOND
  []
  [test_elem_szabab]
    family = SZABAB
    order = FIRST
  []
  [test_elem_l2_lagrange]
    family = L2_LAGRANGE
    order = FIRST
  []
  [test_elem_l2_lagrange_high]
    family = L2_LAGRANGE
    order = SECOND
  []
  [test_elem_l2_hierarchic]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [test_elem_l2_hierarchic_high]
    family = L2_HIERARCHIC
    order = SECOND
  []

  [test_nodal_lagrange]
  []
  [test_nodal_lagrange_high]
    order = SECOND
  []
  [test_nodal_mono]
    family = MONOMIAL
    order = CONSTANT
  []
  [test_nodal_mono_high]
    family = MONOMIAL
    order = SECOND
  []
  [test_nodal_fv]
    type = MooseVariableFVReal
  []
  [test_nodal_xyz]
    family = XYZ
    order = CONSTANT
  []
  [test_nodal_xyz_high]
    family = XYZ
    order = SECOND
  []
  [test_nodal_szabab]
    family = SZABAB
    order = FIRST
  []
  [test_nodal_l2_lagrange]
    family = L2_LAGRANGE
    order = FIRST
  []
  [test_nodal_l2_lagrange_high]
    family = L2_LAGRANGE
    order = SECOND
  []
  [test_nodal_l2_hierarchic]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [test_nodal_l2_hierarchic_high]
    family = L2_HIERARCHIC
    order = SECOND
  []
[]

[AuxKernels]
  # Project from constant monomial
  [base_elem_proj_lagrange]
    type = ProjectionAux
    variable = test_elem_lagrange
    v = base_elem
  []
  [base_elem_proj_lagrange_high]
    type = ProjectionAux
    variable = test_elem_lagrange_high
    v = base_elem
  []
  [base_elem_proj_mono]
    type = ProjectionAux
    variable = test_elem_mono
    v = base_elem
  []
  [base_elem_proj_mono_high]
    type = ProjectionAux
    variable = test_elem_mono_high
    v = base_elem
  []
  [base_elem_proj_fv]
    type = ProjectionAux
    variable = test_elem_fv
    v = base_elem
  []
  [base_elem_proj_xyz]
    type = ProjectionAux
    variable = test_elem_xyz
    v = base_elem
  []
  [base_elem_proj_xyz_high]
    type = ProjectionAux
    variable = test_elem_xyz_high
    v = base_elem
  []
  [base_elem_proj_szabab]
    type = ProjectionAux
    variable = test_elem_szabab
    v = base_elem
  []
  [base_elem_proj_l2_lagrange]
    type = ProjectionAux
    variable = test_elem_l2_lagrange
    v = base_elem
  []
  [base_elem_proj_l2_lagrange_high]
    type = ProjectionAux
    variable = test_elem_l2_lagrange_high
    v = base_elem
  []
  [base_elem_proj_l2_hierarchic]
    type = ProjectionAux
    variable = test_elem_l2_hierarchic
    v = base_elem
  []
  [base_elem_proj_l2_hierarchic_high]
    type = ProjectionAux
    variable = test_elem_l2_hierarchic_high
    v = base_elem
  []

  # Project from constant nodal
  [base_nodal_proj_lagrange]
    type = ProjectionAux
    variable = test_nodal_lagrange
    v = base_nodal
  []
  [base_nodal_proj_lagrange_high]
    type = ProjectionAux
    variable = test_nodal_lagrange_high
    v = base_nodal
  []
  [base_nodal_proj_mono]
    type = ProjectionAux
    variable = test_nodal_mono
    v = base_nodal
  []
  [base_nodal_proj_mono_high]
    type = ProjectionAux
    variable = test_nodal_mono_high
    v = base_nodal
  []
  [base_nodal_proj_fv]
    type = ProjectionAux
    variable = test_nodal_fv
    v = base_nodal
  []
  [base_nodal_proj_xyz]
    type = ProjectionAux
    variable = test_nodal_xyz
    v = base_nodal
  []
  [base_nodal_proj_xyz_high]
    type = ProjectionAux
    variable = test_nodal_xyz_high
    v = base_nodal
  []
  [base_nodal_proj_szabab]
    type = ProjectionAux
    variable = test_nodal_szabab
    v = base_nodal
  []
  [base_nodal_proj_l2_lagrange]
    type = ProjectionAux
    variable = test_nodal_l2_lagrange
    v = base_nodal
  []
  [base_nodal_proj_l2_lagrange_high]
    type = ProjectionAux
    variable = test_nodal_l2_lagrange_high
    v = base_nodal
  []
  [base_nodal_proj_l2_hierarchic]
    type = ProjectionAux
    variable = test_nodal_l2_hierarchic
    v = base_nodal
  []
  [base_nodal_proj_l2_hierarchic_high]
    type = ProjectionAux
    variable = test_nodal_l2_hierarchic_high
    v = base_nodal
  []
[]

[Postprocessors]
  [base_elem_proj_lagrange]
    type = ElementL2Difference
    variable = test_elem_lagrange
    other_variable = base_elem
  []
  [base_elem_proj_lagrange_high]
    type = ElementL2Difference
    variable = test_elem_lagrange_high
    other_variable = base_elem
  []
  [base_elem_proj_mono]
    type = ElementL2Difference
    variable = test_elem_mono
    other_variable = base_elem
  []
  [base_elem_proj_mono_high]
    type = ElementL2Difference
    variable = test_elem_mono_high
    other_variable = base_elem
  []
  [base_elem_proj_fv]
    type = ElementL2Difference
    variable = test_elem_fv
    other_variable = base_elem
  []
  [base_elem_proj_xyz]
    type = ElementL2Difference
    variable = test_elem_xyz
    other_variable = base_elem
  []
  [base_elem_proj_xyz_high]
    type = ElementL2Difference
    variable = test_elem_xyz_high
    other_variable = base_elem
  []
  [base_elem_proj_szabab]
    type = ElementL2Difference
    variable = test_elem_szabab
    other_variable = base_elem
  []
  [base_elem_proj_l2_lagrange]
    type = ElementL2Difference
    variable = test_elem_l2_lagrange
    other_variable = base_elem
  []
  [base_elem_proj_l2_lagrange_high]
    type = ElementL2Difference
    variable = test_elem_l2_lagrange_high
    other_variable = base_elem
  []
  [base_elem_proj_l2_hierarchic]
    type = ElementL2Difference
    variable = test_elem_l2_hierarchic
    other_variable = base_elem
  []
  [base_elem_proj_l2_hierarchic_high]
    type = ElementL2Difference
    variable = test_elem_l2_hierarchic_high
    other_variable = base_elem
  []

  # Project from constant nodal
  [base_nodal_proj_lagrange]
    type = ElementL2Difference
    variable = test_nodal_lagrange
    other_variable = base_nodal
  []
  [base_nodal_proj_lagrange_high]
    type = ElementL2Difference
    variable = test_nodal_lagrange_high
    other_variable = base_nodal
  []
  [base_nodal_proj_mono]
    type = ElementL2Difference
    variable = test_nodal_mono
    other_variable = base_nodal
  []
  [base_nodal_proj_mono_high]
    type = ElementL2Difference
    variable = test_nodal_mono_high
    other_variable = base_nodal
  []
  [base_nodal_proj_fv]
    type = ElementL2Difference
    variable = test_nodal_fv
    other_variable = base_nodal
  []
  [base_nodal_proj_xyz]
    type = ElementL2Difference
    variable = test_nodal_xyz
    other_variable = base_nodal
  []
  [base_nodal_proj_xyz_high]
    type = ElementL2Difference
    variable = test_nodal_xyz_high
    other_variable = base_nodal
  []
  [base_nodal_proj_szabab]
    type = ElementL2Difference
    variable = test_nodal_szabab
    other_variable = base_nodal
  []
  [base_nodal_proj_l2_lagrange]
    type = ElementL2Difference
    variable = test_nodal_l2_lagrange
    other_variable = base_nodal
  []
  [base_nodal_proj_l2_lagrange_high]
    type = ElementL2Difference
    variable = test_nodal_l2_lagrange_high
    other_variable = base_nodal
  []
  [base_nodal_proj_l2_hierarchic]
    type = ElementL2Difference
    variable = test_nodal_l2_hierarchic
    other_variable = base_nodal
  []
  [base_nodal_proj_l2_hierarchic_high]
    type = ElementL2Difference
    variable = test_nodal_l2_hierarchic_high
    other_variable = base_nodal
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
