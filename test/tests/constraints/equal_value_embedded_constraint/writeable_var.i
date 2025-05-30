# TestWriteableNodeElemConstraint writes the nodal id to output_writeable_var1
# This nodal id will match the nodal id output by the vector postprocessor for the regions where the constraint is active

[Mesh]
  file = gold/1D_2D.e
[]

[Variables]
  [phi]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diffusion1]
    type = Diffusion
    variable = phi
  []
[]

[BCs]
  [top_block1]
    type = DirichletBC
    variable = phi
    boundary = 1
    value = 10.0
  []
  [bottom_block1]
    type = DirichletBC
    variable = phi
    boundary = 2
    value = 0.0
  []
[]

[AuxVariables]
  [output_var]
  []
[]

[Constraints]
  [equal_phi1_phi2]
    type = TestWriteableNodeElemConstraint
    secondary = 2
    primary = 1
    penalty = 1e3
    primary_variable = phi
    variable = phi
    output_writeable_var1 = output_var
  []
[]

[VectorPostprocessors]
  [output_var]
    type = NodalValueSampler
    sort_by = id
    block = '2'
    variable = 'output_var'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-8
  l_max_its = 100
  nl_max_its = 10
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  execute_on = 'FINAL'
[]
