# Using ConservativeAdvection with full upwinding
# This demonstrates BCs that introduce no mass into
# the domain but allow it to exit freely.
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
  nx = 100
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u]
    type = FunctionIC
    variable = u
    function = 'if(x<5,x,10-x)'
  [../]
[]

[Kernels]
  [./dot]
    type = MassLumpedTimeDerivative
    variable = u
  [../]
  [./advection]
    type = ConservativeAdvection
    variable = u
    upwinding_type = full
    velocity = '1 0 0'
  [../]
[]

[BCs]
  [./allow_mass_out]
    type = OutflowBC
    boundary = right
    variable = u
    velocity = '1 0 0'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Linear
  dt = 1
  end_time = 10
  l_tol = 1E-14
[]

[Outputs]
  exodus = true
[]

