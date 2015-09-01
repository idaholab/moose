[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Materials]
  [./vecrangecheck]
    type = VecRangeCheckMaterial
    block = 0
    rv3 = '1.1 2.2 3.3'
    iv3 = '1 2 3'
    rvp = '0.1 0.2 0.3 0.4'
    uvg = '2 1'
    lvg = '2 1'
    ivg = '2 1'
    rvg = '2.0 1.0'
    rvl = '0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3'
    rve = ''
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  #kernel_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
[]
