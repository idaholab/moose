[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./t_infinity]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    value = 1000
  [../]
[]

[AuxKernels]
  [./t_infinity]
    type = ConstantAux
    variable = t_infinity
    value = 500
    execute_on = initial
  [../]
[]

[BCs]
  [./right]
    type = CoupledConvectiveHeatFluxBC
    variable = u
    boundary = right
    htc = 10
    T_infinity = t_infinity
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
