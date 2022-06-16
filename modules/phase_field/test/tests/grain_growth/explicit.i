[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 400
  ymin = 0
  ymax = 400
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 1
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
  implicit = false
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalCircleGrainIC]
      radius = 300
      x = 400
      y = 0
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
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
    execute_on = 'initial timestep_end'
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
  scheme = explicit-euler
  solve_type = NEWTON

  # petsc_options_iname = '-pc_type'
  # petsc_options_value = 'bjacobi'
  #
  l_tol = 1.0e-6
  nl_rel_tol = 1.0e-10
  num_steps = 61
  dt = 0.08
[]

[Outputs]
  execute_on = 'initial timestep_end final'
  interval = 20
  exodus = true
[]
