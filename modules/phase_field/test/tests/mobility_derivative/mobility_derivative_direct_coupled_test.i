[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 30
  ymax = 30
  elem_type = QUAD4
[]

[Variables]
  [./c]
    family = HERMITE
    order = THIRD
  [../]
  [./d]
  [../]
[]

[ICs]
  [./c_IC]
    type = SmoothCircleIC
    x1 = 15
    y1 = 15
    radius = 12
    variable = c
    int_width = 3
    invalue = 1
    outvalue = 0
  [../]
  [./d_IC]
    type = BoundingBoxIC
    x1 = 0
    x2 = 15
    y1 = 0
    y2 = 30
    inside = 1.0
    outside = 0.0
    variable = d
  [../]
[]

[Kernels]
  [./c_bulk]
    type = CahnHilliard
    variable = c
    mob_name = M
    f_name = F
    coupled_variables = d
  [../]
  [./c_int]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
    coupled_variables = d
  [../]
  [./c_dot]
    type = TimeDerivative
    variable = c
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

[Materials]
  [./kappa]
    type = GenericConstantMaterial
    prop_names = kappa_c
    prop_values = 2.0
  [../]
  [./mob]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = 'c d'
    expression = if(d>0.001,d,0.001)*if(c<0,0.5,if(c>1,0.5,1-0.5*c^2))
    derivative_order = 2
  [../]
  [./free_energy]
    type = MathEBFreeEnergy
    property_name = F
    c = c
  [../]
  [./d_diff]
    type = GenericConstantMaterial
    prop_names = diffusivity
    prop_values = 1.0
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
  scheme = BDF2

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31      lu      1'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 0.25
  num_steps = 2
[]

[Outputs]
  execute_on = 'timestep_end'
  [./oversample]
    refinements = 2
    type = Exodus
  [../]
[]
