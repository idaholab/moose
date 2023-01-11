# This material tests the kernels ACBarrierFunction and ACKappaFunction for a
# multiphase system.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = -200
  xmax = 200
  ymin = -200
  ymax = 200
  uniform_refine = 0
[]

[Variables]
  [./gr0]
  [../]
  [./gr1]
  [../]
[]

[ICs]
  [./gr0_IC]
    type = BoundingBoxIC
    variable = gr0
    x1 = -80
    y1 = -80
    x2 = 80
    y2 = 80
    inside = 0
    outside = 1
  [../]
  [./gr1_IC]
    type = BoundingBoxIC
    variable = gr1
    x1 = -80
    y1 = -80
    x2 = 80
    y2 = 80
    inside = 1
    outside = 0
  [../]
[]

[Materials]
  [./constants]
    type = GenericConstantMaterial
    prop_names =  'L   gamma E0 E1'
    prop_values = '0.1 1.5   3  1'
  [../]
  [./h0]
    type = DerivativeParsedMaterial
    property_name = h0
    coupled_variables = 'gr0 gr1'
    expression = 'gr0^2 / (gr0^2 + gr1^2)'
    derivative_order = 2
  [../]
  [./h1]
    type = DerivativeParsedMaterial
    property_name = h1
    coupled_variables = 'gr0 gr1'
    expression = 'gr1^2 / (gr0^2 + gr1^2)'
    derivative_order = 2
  [../]
  [./mu]
    type = DerivativeParsedMaterial
    property_name = mu
    coupled_variables = 'gr0 gr1'
    constant_names = 'mag'
    constant_expressions = '16'
    expression = 'mag * (gr0^2 * gr1^2 + 0.1)'
    derivative_order = 2
  [../]
  [./kappa]
    type = DerivativeParsedMaterial
    property_name = kappa
    coupled_variables = 'gr0 gr1'
    material_property_names = 'h0(gr0,gr1) h1(gr0,gr1)'
    constant_names = 'mag0 mag1'
    constant_expressions = '200 100'
    expression = 'h0*mag0 + h1*mag1'
    derivative_order = 2
  [../]
[]

[Kernels]
  [./gr0_time]
    type = TimeDerivative
    variable = gr0
  [../]
  [./gr0_interface]
    type = ACInterface
    variable = gr0
    coupled_variables = 'gr1'
    mob_name = L
    kappa_name = 'kappa'
  [../]
  [./gr0_switching]
    type = ACSwitching
    variable = gr0
    coupled_variables = 'gr1'
    hj_names = 'h0 h1'
    Fj_names = 'E0 E1'
    mob_name = L
  [../]
  [./gr0_multi]
    type = ACGrGrMulti
    variable = gr0
    v = 'gr1'
    mob_name = L
    gamma_names = 'gamma'
  [../]
  [./gr0_barrier]
    type = ACBarrierFunction
    variable = gr0
    mob_name = L
    gamma = gamma
    v = 'gr1'
  [../]
  [./gr0_kappa]
    type = ACKappaFunction
    variable = gr0
    mob_name = L
    kappa_name = kappa
    v = 'gr1'
  [../]

  [./gr1_time]
    type = TimeDerivative
    variable = gr1
  [../]
  [./gr1_interface]
    type = ACInterface
    variable = gr1
    coupled_variables = 'gr0'
    mob_name = L
    kappa_name = 'kappa'
  [../]
  [./gr1_switching]
    type = ACSwitching
    variable = gr1
    coupled_variables = 'gr0'
    hj_names = 'h0 h1'
    Fj_names = 'E0 E1'
    mob_name = L
  [../]
  [./gr1_multi]
    type = ACGrGrMulti
    variable = gr1
    v = 'gr0'
    mob_name = L
    gamma_names = 'gamma'
  [../]
  [./gr1_barrier]
    type = ACBarrierFunction
    variable = gr1
    mob_name = L
    gamma = gamma
    v = 'gr0'
  [../]
  [./gr1_kappa]
    type = ACKappaFunction
    variable = gr1
    mob_name = L
    kappa_name = kappa
    v = 'gr0'
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
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart -sub_ksp_type'
  petsc_options_value = ' asm      ilu          1               31                 preonly'
  nl_max_its = 20
  l_max_its = 30
  l_tol = 1e-4
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  start_time = 0
  num_steps = 3
  dt = 1
[]

[Outputs]
  exodus = true
[]
