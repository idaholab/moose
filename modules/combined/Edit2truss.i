[Mesh]
  type = PeridynamicsMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0.0
  ymin = 0.0
  xmax = 100.0
  ymax = 100.0
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./react_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./react_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./bc_func]
    type = ParsedFunction
    value = t*d
    vars = 'd'
    vals = '1'
  [../]
[]

[BCs]
  [./fixx0]
    type = DirichletBC
    variable = disp_x
    boundary = 0
    value = 0.0
  [../]
  [./fixy0]
    type = DirichletBC
    variable = disp_y
    boundary = 0
    value = 0.0
  [../]
  [./fixz0]
    type = DirichletBC
    variable = disp_z
    boundary = 0
    value = 0.0
  [../]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 1
    function = bc_func
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./fixDummyHex_x]
    type = DirichletBC
    variable = disp_x
    boundary = 100
    value = 0.0
  [../]

  [./fixDummyHex_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./fixDummyHex_z]
    type = DirichletBC
    variable = disp_z
    boundary = 100
    value = 0.0
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    block = 0
    property = axial_stress
    variable = axial_stress
  [../]
  [./e_over_l]
    type = MaterialRealAux
    block = 0
    property = e_over_l
    variable = e_over_l
  [../]
  [./area1]
    type = ConstantAux
    block = 0
    variable = area
    value = 1.0
    execute_on = 'initial timestep_begin'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  solve_type = PJFNK #NEWTON
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  end_time = 1
  line_search = basic
#  petsc_options = '-snes_view -snes_check_jacobian -snes_check_jacobian_view'
  petsc_options_iname = "-pc-type -pc_hypre_type"
  petsc_options_value = "hypre    boomeramg"
[]

[SolidMechanics]
  [./dummyHex]
    block = 100
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Kernels]
  [./solid_x]
    type = StressDivergenceTruss
    block = 0
    variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 0
    area = area
    save_in = react_x
  [../]
  [./solid_y]
    type = StressDivergenceTruss
    block = 0
    variable = disp_y
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 1
    area = area
    save_in = react_y
  [../]
  [./solid_z]
    type = StressDivergenceTruss
    block = 0
    variable = disp_z
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 2
    area = area
    save_in = react_z
  [../]
[]

[Materials]
  [./goo]
    type = Elastic
    block = 100
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 70000
    poissons_ratio = 0.33
  [../]
  [./linelast]
    type = TrussMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
#    thermal_expansion = 0.1
#    t_ref = 0.5
#    temp = temp
  [../]
[]

[Postprocessors]
  [./sum_react_y_0]
    type = NodalSum
    variable = react_y
    boundary = 0
  [../]
  [./sum_react_y_1]
    type = NodalSum
    variable = react_y
    boundary = 1
  [../]
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
