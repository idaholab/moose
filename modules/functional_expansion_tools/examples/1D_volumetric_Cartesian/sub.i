# Basic example coupling a master and sub app in a 1D Cartesian volume.
#
# The master app provides field values to the sub app via Functional Expansions, which then performs
# its calculations. The sub app's solution field values are then transferred back to the master app
# and coupled into the solution of the master app solution.
#
# This example couples Functional Expansions via AuxVariable.
[Mesh]
  type = GeneratedMesh
  dim = 1

  xmin = 0.0
  xmax = 10.0
  nx = 15
[]

# Non-copy transfers only work with AuxVariable, but nothing will be solved without a variable
# defined. The solution is to define an empty variable tha does nothing, but causes MOOSE to solve
# the AuxKernels that we need.
[Variables]
  [./empty]
  [../]
[]

[AuxVariables]
  [./s]
    order = FIRST
    family = LAGRANGE
  [../]
  [./m_in]
    order = FIRST
    family = LAGRANGE
  [../]
[]

# We must have a kernel for every variable, hence this null kernel to match the variable 'empty'
[Kernels]
  [./null_kernel]
    type = NullKernel
    variable = empty
  [../]
[]

[AuxKernels]
  [./reconstruct_m_in]
    type = FunctionSeriesToAux
    function = FX_Basis_Value_Sub
    variable = m_in
  [../]
  [./calculate_s] # Something to make 's' change each time, but allow a converging solution
    type = ParsedAux
    variable = s
    coupled_variables = m_in
    expression = '2*exp(-m_in/0.8)'
  [../]
[]

[Functions]
  [./FX_Basis_Value_Sub]
    type = FunctionSeries
    series_type = Cartesian
    orders = '3'
    physical_bounds = '0.0  10.0'
    x = Legendre
  [../]
[]

[UserObjects]
  [./FX_Value_UserObject_Sub]
    type = FXVolumeUserObject
    function = FX_Basis_Value_Sub
    variable = s
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
