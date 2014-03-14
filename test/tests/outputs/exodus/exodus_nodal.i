##
# \file exodus/exodus_nodal.i
# \example exodus/exodus_nodal.i
# Input file for testing nodal data output
#
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[AuxVariables]
  [./aux0]
    order = SECOND
    family = SCALAR
  [../]
  [./aux1]
    family = SCALAR
    initial_condition = 5
  [../]
  [./aux2]
    family = SCALAR
    initial_condition = 10
  [../]
  [./aux3]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = CoefDiffusion
    variable = v
    coef = 2
  [../]
[]

[BCs]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 3
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 2
  [../]
[]

[Postprocessors]
  [./num_vars]
    type = NumVars
  [../]
  [./num_aux]
    type = NumVars
    system = auxiliary
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

##! [MultipleOutputBlocks]
[Outputs]
  output_initial = false                # common output_initial
  [./exodus]
    # Setup the output system to output only aux2 and aux3 as nodal variables
    type = Exodus
    hide = 'u v aux0 aux1'              # disables variables
    output_postprocessors = false       # disables postprocessors
    output_scalar_variables = false     # disables scalar variables
    output_elemental_variables = false  # disables elemental variables
    scalar_as_nodal = true              # converts aux2 to nodal
    elemental_as_nodal = true           # converts aux3 to nodal
    output_nodal_variables = true       # enable nodal variables (optional, this is the default)
  [../]
  [./screen]
    # Setup the screen output
    type = Console
    output_input = true                 # override the common parameter for this output object
    nonlinear_residuals =  true         # enable printing non-linear residuals (optional, this is the default)
    linear_residuals = false            # disable printing linear residuals
    perf_log = true                     # display performance log
  [../]
[]
##! [MultipleOutputBlocks]

[ICs]
  [./aux0_IC]
    variable = aux0
    values = '12 13'
    type = ScalarComponentIC
  [../]
[]
