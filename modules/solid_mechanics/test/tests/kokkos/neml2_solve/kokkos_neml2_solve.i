# The problem of host_neml2_solve.i, with the displacement solved through the Kokkos assembly and the
# constitutive update evaluated by NEML2 on the device, against the same gold.
#
# A single LAGRANGE_VEC variable carries the displacement. Its Kokkos small strain is gathered into
# NEML2, and the stress and its derivative with respect to the strain are retrieved into the Kokkos
# properties the stress divergence kernel reads, so both the residual and the Jacobian come from
# NEML2 without passing through host memory. The model is stateful, and its old state is gathered from
# the Kokkos properties the corresponding outputs are retrieved into, as in the neml2_gather test.
#
# The small strain formulation evaluates the strain on the undisplaced mesh, so no displaced mesh is
# created. auto_output = false keeps the NEML2 action from adding the non-Kokkos retrievers, which
# would copy every output to the host.
#
# The displacement components are extracted into auxiliary variables only so that the reported
# norms carry the same names as in the reference input, letting both inputs share one gold file.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 2
    nx = 4
    ny = 2
    nz = 2
  []
[]

[Variables]
  [disp]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [stress_divergence]
    type = KokkosSymmetricVectorStressDivergence
    variable = disp
  []
[]

[Functions]
  [pull_x]
    type = KokkosParsedFunction
    expression = '5e-5 * t'
  []
  [pull_y]
    type = KokkosParsedFunction
    expression = '2.5e-5 * t'
  []
  [position_weight]
    type = ParsedFunction
    expression = 'x + 2 * y + 3 * z'
  []
[]

[BCs]
  [fixed]
    type = KokkosVectorDirichletBC
    variable = disp
    boundary = left
    values = '0 0 0'
  []
  [pull]
    type = KokkosVectorFunctionDirichletBC
    variable = disp
    boundary = right
    function_x = pull_x
    function_y = pull_y
  []
[]

[NEML2]
  input = '../../neml2/plasticity/isoharden_neml2.i'
  [all]
    model = 'model'
    device = 'cuda'
    output_device = 'cuda'
    auto_output = false
    # Every per-quadrature-point input is supplied by a Kokkos gatherer below
    input_kernels = 'neml2_strain plastic_strain~1 equivalent_plastic_strain~1 flow_rate~1'
  []
[]

[UserObjects]
  [neml2_strain]
    type = KokkosMOOSESymmetricRankTwoTensorToNEML2
    from_moose = 'mechanical_strain'
    to_neml2 = 'neml2_strain'
  []
  # The old state, read from the Kokkos properties the corresponding outputs are retrieved into.
  # Requesting the old value is what makes those properties stateful.
  [plastic_strain~1]
    type = KokkosMOOSEOldSymmetricRankTwoTensorToNEML2
    from_moose = 'plastic_strain'
    to_neml2 = 'plastic_strain~1'
  []
  [equivalent_plastic_strain~1]
    type = KokkosMOOSEOldRealToNEML2
    from_moose = 'equivalent_plastic_strain'
    to_neml2 = 'equivalent_plastic_strain~1'
  []
  [flow_rate~1]
    type = KokkosMOOSEOldRealToNEML2
    from_moose = 'flow_rate'
    to_neml2 = 'flow_rate~1'
  []
[]

[Materials]
  [strain]
    type = KokkosSymmetricComputeSmallStrain
    displacements = 'disp'
  []
  # The stress and tangent the stress divergence kernel reads. The tangent of a plastic model varies
  # along the batch, so it is stored per quadrature point.
  [stress]
    type = KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'neml2_stress'
    to_moose = 'stress'
  []
  [jacobian]
    type = KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'neml2_stress'
    neml2_input_derivative = 'neml2_strain'
    to_moose = 'Jacobian_mult'
  []
  # The stateful outputs, retrieved into Kokkos properties so the old state can be gathered on the
  # device by the user objects above
  [plastic_strain]
    type = KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'plastic_strain'
    to_moose = 'plastic_strain'
  []
  [equivalent_plastic_strain]
    type = KokkosNEML2ToMOOSERealMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'equivalent_plastic_strain'
    to_moose = 'equivalent_plastic_strain'
  []
  [flow_rate]
    type = KokkosNEML2ToMOOSERealMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'flow_rate'
    to_moose = 'flow_rate'
  []
[]

[AuxVariables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [eqps]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [disp_x]
    type = VectorVariableComponentAux
    variable = disp_x
    vector_variable = disp
    component = x
  []
  [disp_y]
    type = VectorVariableComponentAux
    variable = disp_y
    vector_variable = disp
    component = y
  []
  [disp_z]
    type = VectorVariableComponentAux
    variable = disp_z
    vector_variable = disp
    component = z
  []
  [stress_xx]
    type = KokkosSymmetricRankTwoComponentAux
    variable = stress_xx
    property = 'stress'
    component = 0
    execute_on = 'TIMESTEP_END'
  []
  [eqps]
    type = KokkosMaterialRealAux
    variable = eqps
    property = 'equivalent_plastic_strain'
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 4
  dt = 1
  # The Kokkos vector Dirichlet conditions are imposed through the residual rather than preset, so each
  # step starts with a residual made only of the boundary rows, in units of displacement. The first
  # Newton update satisfies them and leaves an equilibrium residual in units of force, which a
  # backtracking line search mistakes for an increase and rejects.
  line_search = none
[]

[Postprocessors]
  [disp_x_l2]
    type = ElementL2Norm
    variable = disp_x
  []
  [disp_y_l2]
    type = ElementL2Norm
    variable = disp_y
  []
  [disp_z_l2]
    type = ElementL2Norm
    variable = disp_z
  []
  # Weighting by position makes this integral sensitive to the stress field being permuted among
  # elements, which a plain integral is not
  [stress_moment]
    type = ElementIntegralFunctorPostprocessor
    functor = stress_xx
    prefactor = position_weight
  []
  [plastic_strain_integral]
    type = ElementIntegralVariablePostprocessor
    variable = eqps
  []
  [max_equivalent_plastic_strain]
    type = ElementExtremeValue
    variable = eqps
  []
[]

[Outputs]
  csv = true
[]
