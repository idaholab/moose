# 2D test of advection with full upwinding
# Note there are no overshoots or undershoots
# but there is numerical diffusion.
# The center of the blob advects with the correct velocity
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_blob]
    type = FunctionIC
    variable = u
    function = 'if(x<0.2,if(y<0.2,1,0),0)'
  [../]
[]

[Kernels]
  [./udot]
    type = MassLumpedTimeDerivative
    variable = u
  [../]
  [./advection]
    type = ConservativeAdvection
    variable = u
    upwinding_type = full
    velocity = '2 1 0'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = LINEAR
  dt = 0.01
  end_time = 0.1
  l_tol = 1E-14
[]

[Outputs]
  exodus = true
[]
