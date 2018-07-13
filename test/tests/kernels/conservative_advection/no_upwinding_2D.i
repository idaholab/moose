# 2D test of advection with no upwinding
# Note there are overshoots or undershoots
# but numerical diffusion is minimized.
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
    type = TimeDerivative
    variable = u
  [../]
  [./advection]
    type = ConservativeAdvection
    variable = u
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
