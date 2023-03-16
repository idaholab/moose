[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmax = 30.0
  ymax = 30.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
  [../]
  [./w]
  [../]
  [./d]
  [../]
[]

[ICs]
  [./c_IC]
    type = CrossIC
    x1 = 0.0
    x2 = 30.0
    y1 = 0.0
    y2 = 30.0
    variable = c
  [../]
  [./d_IC]
    type = BoundingBoxIC
    x1 = 0.0
    x2 = 15.0
    y1 = 0.0
    y2 = 30.0
    inside = 1.0
    outside = 0.0
    variable = d
  [../]
[]

[Kernels]
  [./cres]
    type = SplitCHParsed
    variable = c
    kappa_name = kappa_c
    w = w
    f_name = F
  [../]
  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
    coupled_variables = 'c d'
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./d_dot]
    type = TimeDerivative
    variable = d
  [../]
  [./d_diff]
    type = MatDiffusion
    variable = d
    diffusivity = diffusivity
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./kappa]
    type = GenericConstantMaterial
    prop_names = 'kappa_c'
    prop_values = '2.0'
  [../]
  [./mob]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = 'c d'
    expression = 'if(d>0.001,d,0.001)*(1-0.5*c^2)'
    outputs = exodus
    derivative_order = 1
  [../]
  [./free_energy]
    type = MathEBFreeEnergy
    property_name = F
    c = c
  [../]
  [./d_diff]
    type = GenericConstantMaterial
    prop_names = diffusivity
    prop_values = 0.1
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
  scheme = 'BDF2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31      lu      1'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 10.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
