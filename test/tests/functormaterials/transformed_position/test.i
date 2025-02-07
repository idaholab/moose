[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 4
    xmax = 10
    ymax = 1
  []
[]

# These two functions naturally stay in the mesh bounds, they just do a symmetry around the
# middle of the mesh
# These must hit nodes exactly when evaluated at node
[Functions]
  [f1]
    type = ParsedFunction
    expression = '10 - x'
  []
  [f2]
    type = ParsedFunction
    expression = '1 - y'
  []
[]

[AuxVariables]
  [base]
    type = MooseVariableFVReal
    [AuxKernel]
      type = ParsedAux
      expression = 'x*log(x) + 4*y*y*y'
      use_xyzt = true
    []
  []
  [output_nodal_evals]
  []
  [output_elem_evals]
    type = MooseVariableFVReal
  []
  [output_elem_qp_evals]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[FunctorMaterials]
  [translate]
    type = ADFunctorTransformFunctorMaterial
    prop_names = 'transformed'
    prop_values = 'base'
    x_functor = 'f1'
    y_functor = 'f2'
    z_functor = '0'
    block = '0'
  []
[]

[AuxKernels]
  [proj_at_nodal]
    type = FunctorAux
    variable = output_nodal_evals
    functor = 'transformed'
    # Necessary to make FunctorAux call the nodal evaluation with a certain block
    block = '0'
    execute_on = 'TIMESTEP_END'
  []
  [proj_at_elem]
    type = FunctorAux
    variable = output_elem_evals
    functor = 'transformed'
    execute_on = 'TIMESTEP_END'
  []
  [proj_at_elem_qp]
    type = FunctorAux
    variable = output_elem_qp_evals
    functor = 'transformed'
  []
[]

[VectorPostprocessors]
  [elem_point_evals]
    type = PositionsFunctorValueSampler
    positions = 'elem_positions'
    functors = 'transformed'
    sort_by = 'id'
  []
[]

[Positions]
  [elem_positions]
    type = ElementCentroidPositions
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  hide = 'base'
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
