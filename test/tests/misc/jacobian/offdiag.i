[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./s]
    [./InitialCondition]
      type = FunctionIC
      function = sin(10*x+y)
    [../]
  [../]
  [./t]
    [./InitialCondition]
      type = FunctionIC
      function = sin(13*y+x)
    [../]
  [../]
[]

[Kernels]
  [./diffs]
    type = WrongJacobianDiffusion
    variable = s
    coupled = t
  [../]
  [./difft]
    type = WrongJacobianDiffusion
    variable = t
    coupled = s
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
