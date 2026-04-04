# Minimal Working Example to reproduce an array variable + nodal constraint issue
# This test case creates:
# 1. A scalar nonlinear variable (T_solid)
# 2. An array auxiliary variable (Nus) with 2 components
# 3. Nodal constraints that couple T_solid
#
# Expected behavior: Should run without error
# Actual behavior: Crashes with DOF indices error during constraint evaluation
# The bug was that the array variable dofs were not all gathered at each node

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = 0
    xmax = 1.0
  []

  [add_bdy]
    type = ExtraNodesetGenerator
    input = gen
    coord = '0.5 0 0'
    new_boundary = 'fluid_solid'
  []
[]

[Functions]
  [htc_func]
    type = ConstantFunction
    value = 1000
  []
  [re_func]
    type = ConstantFunction
    value = 5000
  []
[]

[Variables]
  # Solid temperature
  [T_solid]
    order = FIRST
    family = LAGRANGE
    initial_condition = 500
  []
[]

[AuxVariables]
  # Regular scalar auxiliary variable
  [htc]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000
  []

  # ARRAY AUXILIARY VARIABLE - This is what causes the issue
  # Component 0: Heat transfer coefficient
  # Component 1: Reynolds number
  [Nus]
    order = FIRST
    family = LAGRANGE
    components = 2
    initial_condition = '1000 5000'
  []
[]

[AuxKernels]
  # Compute array variable using functions
  [compute_Nus]
    type = FunctionArrayAux
    variable = Nus
    functions = 'htc_func re_func'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  # Extract HTC (component 0) from array variable
  [extract_htc]
    type = ArrayVariableComponent
    variable = htc
    array_variable = Nus
    component = 0
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Kernels]
  # Simple diffusion for solid
  [heat_solid]
    type = Diffusion
    variable = T_solid
  []
[]

[Constraints]
  # These nodal constraints copy T_solid to Tw at interface nodes
  # The crash occurs when MOOSE tries to reinitialize neighbor nodes
  # and encounters the Nus array auxiliary variable

  [copy_Tw_node5]
    type = LinearNodalConstraint
    variable = T_solid
    primary = '2'
    secondary_node_set = '2'
    penalty = 1e6
    weights = 1
  []
[]

[BCs]
  [left_bc]
    type = DirichletBC
    variable = T_solid
    boundary = 'left'
    value = 600
  []

  [right_bc]
    type = DirichletBC
    variable = T_solid
    boundary = 'right'
    value = 400
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-8
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

