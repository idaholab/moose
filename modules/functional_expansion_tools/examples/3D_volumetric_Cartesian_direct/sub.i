# Derived from the example '3D_volumetric_Cartesian' with the following differences:
#
#   1) The coupling is performed via BodyForce instead of the
#      FunctionSeriesToAux+CoupledForce approach
[Mesh]
  type = GeneratedMesh
  dim = 3

  xmin = 0.0
  xmax = 10.0
  nx = 15

  ymin = 1.0
  ymax = 11.0
  ny = 25

  zmin = 2.0
  zmax = 12.0
  nz = 35
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
    orders = '3   4   5'
    physical_bounds = '0.0  10.0    1.0 11.0    2.0 12.0'
    x = Legendre
    y = Legendre
    z = Legendre
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
