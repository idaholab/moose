#
# This test ensures that a flat grain boundary does not move
# under a temperature gradient using the normal grain growth model
#
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 20
  xmax = 1000
  ymax = 500
  elem_type = QUAD
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
[]

[Modules]
  [./PhaseField]
    [./GrainGrowth]
      coupled_variables = T
      variable_mobility = true
    [../]
  [../]
[]


[Functions]
  [./TGradient]
    type = ParsedFunction
    expression = '450 + 0.1*x'
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalBoundingBoxIC]
      x1 = 0.0
      x2 = 500.0
      y1 = 0.0
      y2 = 500.0
    [../]
  [../]
[]

[AuxVariables]
  [./T]
  [../]
[]

[AuxKernels]
  [./Tgrad]
    type = FunctionAux
    variable = T
    function = TGradient
  [../]
[]

[Materials]
  [./Copper]
    type = GBEvolution
    T = T # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
  [../]
[]

[Postprocessors]
  [./gr0_area]
    type = ElementIntegralVariablePostprocessor
    variable = gr0
    execute_on = 'initial TIMESTEP_END'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 10
  dt = 100.0
[]

[Outputs]
  exodus = true
[]
