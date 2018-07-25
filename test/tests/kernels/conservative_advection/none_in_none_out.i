# Using ConservativeAdvection with full upwinding
# This demonstrates BCs (no BCs) that allow no mass to
# enter or exit the domain.
# Total mass remains constant and the pulse advects
# with the correct velocity
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
  nx = 10
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

[Postprocessors]
  [./total_mass]
    type = VariableInnerProduct
    variable = u
    second_variable = 1
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
  csv = true
[]

