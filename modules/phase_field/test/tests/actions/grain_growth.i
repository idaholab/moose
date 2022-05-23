[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmax = 400
  ymax = 400
  elem_type = QUAD
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
[]

[Modules]
  [./PhaseField]
    [./GrainGrowth]
      variable_mobility = false
    [../]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalCircleGrainIC]
      radius = 300
      x = 400
      y = 0
      int_width = 60
    [../]
  [../]
[]

[Materials]
  [./Copper]
    type = GBEvolution
    T = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 #m^4/(Js) from Schoenfelder 1997
    Q = 0.23 #Migration energy in eV
    GBenergy = 0.708 #GB energy in J/m^2
  [../]
[]

[Postprocessors]
  [./gr1area]
    type = ElementIntegralVariablePostprocessor
    variable = gr1
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = 'NEWTON'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  num_steps = 5
  dt = 80.0
[]

[Outputs]
  exodus = true
[]
