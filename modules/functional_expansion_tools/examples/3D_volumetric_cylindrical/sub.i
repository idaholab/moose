# Basic example coupling a master and sub app in a 3D cylindrical mesh from an input file
#
# The master app provides field values to the sub app via Functional Expansions, which then performs
# its calculations. The sub app's solution field values are then transferred back to the master app
# and coupled into the solution of the master app solution.
#
# This example couples Functional Expansions via AuxVariable, the recommended approach.
#
# Note: this problem is not light, and may take a few minutes to solve.
[Mesh]
  type = FileMesh
  file = cyl-tet.e
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
    series_type = CylindricalDuo
    orders = '5   3' # Axial first, then (r, t) FX
    physical_bounds = '-2.5 2.5   0 0 1' # z_min z_max   x_center y_center radius
    z = Legendre # Axial in z
    disc = Zernike # (r, t) default to unit disc in x-y plane
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
