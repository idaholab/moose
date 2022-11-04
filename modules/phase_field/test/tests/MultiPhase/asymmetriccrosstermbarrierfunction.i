[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmin = 0
  xmax = 9
[]

[Functions]
  [./func1]
    type = ParsedFunction
    expression = 'il:=x-7; ir:=2-x; if(x<1, 1,
                               if(x<2, 0.5-0.5*cos(ir*pi),
                               if(x<7, 0,
                               if(x<8, 0.5-0.5*cos(il*pi),
                               1))))'
  [../]
  [./func2]
    type = ParsedFunction
    expression = 'il:=x-1; ir:=5-x; if(x<1, 0,
                               if(x<2, 0.5-0.5*cos(il*pi),
                               if(x<4, 1,
                               if(x<5, 0.5-0.5*cos(ir*pi),
                               0))))'
  [../]
  [./func3]
    type = ParsedFunction
    expression = 'il:=x-4; ir:=8-x; if(x<4, 0,
                               if(x<5, 0.5-0.5*cos(il*pi),
                               if(x<7, 1,
                               if(x<8, 0.5-0.5*cos(ir*pi),
                               0))))'
  [../]
[]

[AuxVariables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = func1
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = func2
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = func3
    [../]
  [../]
[]

[Materials]
  [./symmetriccrosstermbarrier_low]
    type = AsymmetricCrossTermBarrierFunctionMaterial
    etas     = 'eta1 eta2 eta3'
    hi_names = 'h1   h2   h3'
    W_ij = '0   1   2.2
            1   0   3.1
            2.2 3.1 0'
    function_name = gsl
    g_order = LOW
    outputs = exodus
  [../]
  [./asymmetriccrosstermbarrier_low]
    type = AsymmetricCrossTermBarrierFunctionMaterial
    etas     = 'eta1 eta2 eta3'
    hi_names = 'h1   h2   h3'
    W_ij = ' 0    1.2 5.2
             0.8  0   2.1
            -0.8 4.1  0'
    function_name = gal
    g_order = LOW
    outputs = exodus
  [../]

  [./asymmetriccrosstermbarrie_simple]
    type = AsymmetricCrossTermBarrierFunctionMaterial
    etas     = 'eta1 eta2 eta3'
    hi_names = 'h1   h2   h3'
    W_ij = '0   1.2   3.2
            0.8   0   2.1
            1.2 4.1 0'
    function_name = gas
    g_order = SIMPLE
    outputs = exodus
  [../]

  [./switch1]
    type = SwitchingFunctionMaterial
    function_name = h1
    eta = eta1
  [../]
  [./switch2]
    type = SwitchingFunctionMaterial
    function_name = h2
    eta = eta2
  [../]
  [./switch3]
    type = SwitchingFunctionMaterial
    function_name = h3
    eta = eta3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
  execute_on = final
[]
