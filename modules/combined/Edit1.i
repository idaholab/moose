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
  [./temp]
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
  [./bond_status]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./failure_index]
    order = FIRST
    family = LAGRANGE
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
  [./bc_func1]
    type = ParsedFunction
    value = t*d
    vars = 'd'
    vals = '1'
  [../]
  [./bc_func2]
    type = ParsedFunction
    value = t*T
    vars = 'T'
    vals = '100'
  [../]
[]

[BCs]

  [./inlet_temperature]
    type = DirichletBC
    variable = temp
    boundary = 0
    value = 0
  [../]
#  [./outlet_temperature]
#    type = FunctionPresetBC
#    variable = temp
#    boundary = 1
#    function = bc_func2
#  [../]
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
#  [./fixx1]
#    type = DirichletBC
#    variable = disp_x
#    boundary = 1
#    value = 0.0
#  [../]
#  [./fixy1]
#    type = FunctionPresetBC
#    variable = disp_y
#    boundary = 1
#    function = bc_func1
#  [../]
#  [./fixz1]
#    type = DirichletBC
#    variable = disp_z
#    boundary = 1
#    value = 0.0
#  [../]

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
  [./fixDummyHex_temp]
    type = DirichletBC
    variable = temp
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
  [./bond_status]
    type = MaterialRealAux
    block = 0
    property = bond_status
    variable = bond_status
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  solve_type = NEWTON #PJFNK
  nl_max_its = 500
  nl_rel_tol = 1e-4
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
    type = StressDivergenceTrussPD
    block = 0
    variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
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
    temp = temp
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
    temp = temp
    component = 2
    save_in = react_z
  [../]
#  [./diff]
#    type = Diffusion
#    variable = temp
#    coef = 0.1
#  [../]
  [./heatconduction]
    type = HeatConduction
    block = 0
    variable = temp
  [../]
  [./heatsource]
    type = HeatSourcePD
    block = 0
    variable = temp
    PowerDensity = 100.0;
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
    temp = temp
    youngs_modulus = 70000
    poissons_ratio = 0.33
    MeshSpacing = 1.0                # MeshSpacing = (xmax - xmin) / nx
    ThicknessPerLayer = 1.0          # For 2D case, ThicknessPerLayer needs a value; For 3D case, ThicknessPerLayer = MeshSpacing
    CriticalStretch = 0.001
    reference_temp = 0.0
    thermal_expansion = 0.000001
    thermal_conductivity = 100.0
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
