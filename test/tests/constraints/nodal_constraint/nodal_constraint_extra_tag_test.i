# Test that NodalConstraint contributions to extra_vector_tags
# are correctly written to the tagged system vector.
# This tests that TagVectorAux can read the constraint's contribution
# from the tagged vector.

[Mesh]
  file = 2-lines.e
  allow_renumbering = false
[]

[Problem]
  extra_tag_vectors = ref
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [react_u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = ref
  []
[]

[AuxKernels]
  [react_u_aux]
    type = TagVectorAux
    variable = react_u
    v = u
    vector_tag = ref
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 3
  []
[]

[Constraints]
  [c1]
    type = EqualValueNodalConstraint
    variable = u
    primary = 0
    secondary = 4
    penalty = 100000
    extra_vector_tags = ref
  []
[]

[Postprocessors]
  # Sum of react_u on all nodes - if constraint tags work,
  # the constraint contribution should appear in react_u
  [react_u_at_primary]
    type = NodalVariableValue
    variable = react_u
    nodeid = 0
  []
  [react_u_at_secondary]
    type = NodalVariableValue
    variable = react_u
    nodeid = 4
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
[]
