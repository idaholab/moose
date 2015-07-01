[Mesh]
  type = PeridynamicsMesh
  dim = 2
  nx = 50
  ny = 50
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
  [./axial_force]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stiff_elem]
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
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 1.0
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
  [./axial_force]
    type = MaterialRealAux
    block = 0
    property = axial_force
    variable = axial_force
  [../]
  [./stiff_elem]
    type = MaterialRealAux
    block = 0
    property = stiff_elem
    variable = stiff_elem
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
  solve_type = PJFNK
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  end_time = 1
  line_search = basic
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
    type = StressDivergenceTrussPD
    block = 0
    variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 0
    save_in = react_x
  [../]
  [./solid_y]
    type = StressDivergenceTrussPD
    block = 0
    variable = disp_y
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 1
    save_in = react_y
  [../]
  [./solid_z]
    type = StressDivergenceTrussPD
    block = 0
    variable = disp_z
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    component = 2
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
    type = PeridynamicBond
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 70000
    poissons_ratio = 0.33
    MeshSpacing = 2.0                # MeshSpacing = (xmax - xmin) / nx;
    ThicknessPerLayer = 1.0          # For 2D case, ThicknessPerLayer needs a value; For 3D case, ThicknessPerLayer = MeshSpacing
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