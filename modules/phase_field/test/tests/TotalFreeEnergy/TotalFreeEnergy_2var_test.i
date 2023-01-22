[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 2
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalCircleGrainIC]
      radius = 333.333
      x = 500
      y = 500
      int_width = 60
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./gr0dot]
    type = TimeDerivative
    variable = gr0
  [../]
  [./gr0bulk]
    type = AllenCahn
    variable = gr0
    f_name = F
    coupled_variables = gr1
  [../]
  [./gr0int]
    type = ACInterface
    variable = gr0
    kappa_name = kappa_op
  [../]
  [./gr1dot]
    type = TimeDerivative
    variable = gr1
  [../]
  [./gr1bulk]
    type = AllenCahn
    variable = gr1
    f_name = F
    coupled_variables = gr0
  [../]
  [./gr1int]
    type = ACInterface
    variable = gr1
    kappa_name = kappa_op
  [../]
[]

[AuxKernels]
  [./BndsCalc]
    type = BndsCalcAux
    variable = bnds
  [../]
  [./local_free_energy]
    type = TotalFreeEnergy
    variable = local_energy
    kappa_names = 'kappa_op kappa_op'
    interfacial_vars = 'gr0 gr1'
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./Copper]
    type = GBEvolution
    T = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    coupled_variables = 'gr0 gr1'
    material_property_names = 'mu gamma_asymm'
    expression = 'mu*( gr0^4/4.0 - gr0^2/2.0 + gr1^4/4.0 - gr1^2/2.0 + gamma_asymm*gr0^2*gr1^2) + 1.0/4.0'
    derivative_order = 2
    enable_jit = true
  [../]
[]

[Postprocessors]
  [./total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
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
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 30
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 7
  dt = 80.0
  [./Adaptivity]
    initial_adaptivity = 2
    refine_fraction = 0.8
    coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
