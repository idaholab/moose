[Mesh]
  type = PeridynamicsMesh
  dim = 2
  shape = 2               # 1. Rectangular, 2. Disk.
  nr = 10
  R = 4.1
  nx = 10
  ny = 10
  nz = 10
  xmin = 0.0
  xmax = 100.0
  ymin = 0.0
  ymax = 100.0
  zmin = 0.0
  zmax = 100.0
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
    initial_condition = 300.0
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
  [./bond_stretch]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./critical_stretch]
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
  [./nodal_power]
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
  [./power_bc]
    type = PiecewiseLinear
    scale_factor = 1.0 #Convert from W/m to W/m3 (1/pi/(.0041m)^2)
    x = '0,  1e4,    1e8'
    y = '0, 473.39, 473.39'
  [../]
  [./temp_bc]
    type = PiecewiseLinear
    xy_data = '0.0000000E+000,  3.0000000E+002
               1.0000000E+002,  5.7602620E+002
               2.0000000E+002,  5.8868136E+002
               3.0000000E+002,  5.9354501E+002
               4.0000000E+002,  5.9803698E+002
               5.0000000E+002,  6.0246166E+002
               6.0000000E+002,  6.0683317E+002
               7.0000000E+002,  6.1115311E+002
               8.0000000E+002,  6.1542251E+002
               9.0000000E+002,  6.1964236E+002
               1.0000000E+003,  6.2381358E+002
               1.1000000E+003,  6.2793709E+002
               1.2000000E+003,  6.3201376E+002
               1.3000000E+003,  6.3604445E+002
               1.4000000E+003,  6.4002998E+002
               1.5000000E+003,  6.4397114E+002
               1.6000000E+003,  6.4786872E+002
               1.7000000E+003,  6.5172346E+002
               1.8000000E+003,  6.5553611E+002
               1.9000000E+003,  6.5930737E+002
               2.0000000E+003,  6.6303796E+002
               2.1000000E+003,  6.6672853E+002
               2.2000000E+003,  6.7037976E+002
               2.3000000E+003,  6.7399234E+002
               2.4000000E+003,  6.7756681E+002
               2.5000000E+003,  6.8110388E+002
               2.6000000E+003,  6.8460411E+002
               2.7000000E+003,  6.8806812E+002
               2.8000000E+003,  6.9149648E+002
               2.9000000E+003,  6.9488976E+002
               3.0000000E+003,  6.9824854E+002
               3.1000000E+003,  7.0157338E+002
               3.2000000E+003,  7.0486476E+002
               3.3000000E+003,  7.0812322E+002
               3.4000000E+003,  7.1134929E+002
               3.5000000E+003,  7.1454345E+002
               3.6000000E+003,  7.1770619E+002
               3.7000000E+003,  7.2083798E+002
               3.8000000E+003,  7.2393919E+002
               3.9000000E+003,  7.2701042E+002
               4.0000000E+003,  7.3005203E+002
               4.1000000E+003,  7.3306446E+002
               4.2000000E+003,  7.3604808E+002
               4.3000000E+003,  7.3900325E+002
               4.4000000E+003,  7.4193037E+002
               4.5000000E+003,  7.4482980E+002
               4.6000000E+003,  7.4770188E+002
               4.7000000E+003,  7.5054697E+002
               4.8000000E+003,  7.5336536E+002
               4.9000000E+003,  7.5615738E+002
               5.0000000E+003,  7.5892332E+002
               5.1000000E+003,  7.6166351E+002
               5.2000000E+003,  7.6437809E+002
               5.3000000E+003,  7.6706738E+002
               5.4000000E+003,  7.6973168E+002
               5.5000000E+003,  7.7237121E+002
               5.6000000E+003,  7.7498620E+002
               5.7000000E+003,  7.7757687E+002
               5.8000000E+003,  7.8014344E+002
               5.9000000E+003,  7.8268613E+002
               6.0000000E+003,  7.8520510E+002
               6.1000000E+003,  7.8770060E+002
               6.2000000E+003,  7.9017276E+002
               6.3000000E+003,  7.9262179E+002
               6.4000000E+003,  7.9504785E+002
               6.5000000E+003,  7.9745112E+002
               6.6000000E+003,  7.9983176E+002
               6.7000000E+003,  8.0218986E+002
               6.8000000E+003,  8.0452571E+002
               6.9000000E+003,  8.0683937E+002
               7.0000000E+003,  8.0913095E+002
               7.1000000E+003,  8.1140067E+002
               7.2000000E+003,  8.1364861E+002
               7.3000000E+003,  8.1587485E+002
               7.4000000E+003,  8.1807958E+002
               7.5000000E+003,  8.2026287E+002
               7.6000000E+003,  8.2242480E+002
               7.7000000E+003,  8.2456544E+002
               7.8000000E+003,  8.2668483E+002
               7.9000000E+003,  8.2047428E+002
               8.0000000E+003,  8.1649166E+002
               8.1000000E+003,  8.1519224E+002
               8.2000000E+003,  8.1470729E+002
               8.3000000E+003,  8.1459804E+002
               8.4000000E+003,  8.1471836E+002
               8.5000000E+003,  8.1499084E+002
               8.6000000E+003,  8.1536815E+002
               8.7000000E+003,  8.1582063E+002
               8.8000000E+003,  8.1632844E+002
               8.9000000E+003,  8.1687609E+002
               9.0000000E+003,  8.1745282E+002
               9.1000000E+003,  8.1805006E+002
               9.2000000E+003,  8.1866094E+002
               9.3000000E+003,  8.1927988E+002
               9.4000000E+003,  8.1990224E+002
               9.5000000E+003,  8.2052362E+002
               9.6000000E+003,  8.2114187E+002
               9.7000000E+003,  8.2175388E+002
               9.8000000E+003,  8.2235750E+002
               9.9000000E+003,  8.2295103E+002
               1.0000000E+004,  8.2353311E+002
               1.0100000E+004,  8.2350435E+002
               1.0200000E+004,  8.2344182E+002
               1.0300000E+004,  8.2338339E+002
               1.0400000E+004,  8.2332938E+002
               1.0500000E+004,  8.2327904E+002
               1.0600000E+004,  8.2323176E+002
               1.0700000E+004,  8.2318705E+002
               1.0800000E+004,  8.2314457E+002
               1.0900000E+004,  8.2310404E+002
               1.1000000E+004,  8.2306521E+002
               1.1100000E+004,  8.2302790E+002
               1.1200000E+004,  8.2299193E+002
               1.1300000E+004,  8.2295719E+002
               1.1400000E+004,  8.2292354E+002
               1.1500000E+004,  8.2289090E+002
               1.1600000E+004,  8.2285919E+002
               1.1700000E+004,  8.2282831E+002
               1.1800000E+004,  8.2279822E+002
               1.1900000E+004,  8.2276885E+002
               1.2000000E+004,  8.2274015E+002
               1.2100000E+004,  8.2271209E+002
               1.2200000E+004,  8.2268460E+002
               1.2300000E+004,  8.2265767E+002
               1.2400000E+004,  8.2263126E+002
               1.2500000E+004,  8.2260534E+002
               1.2600000E+004,  8.2257988E+002
               1.2700000E+004,  8.2255485E+002
               1.2800000E+004,  8.2253024E+002
               1.2900000E+004,  8.2250602E+002
               1.3000000E+004,  8.2248217E+002
               1.3100000E+004,  8.2245868E+002
               1.3200000E+004,  8.2243553E+002
               1.3300000E+004,  8.2241270E+002
               1.3400000E+004,  8.2239019E+002
               1.3500000E+004,  8.2236797E+002
               1.3600000E+004,  8.2234604E+002
               1.3700000E+004,  8.2232439E+002
               1.3800000E+004,  8.2230300E+002
               1.3900000E+004,  8.2228186E+002
               1.4000000E+004,  8.2226096E+002
               1.4100000E+004,  8.2224031E+002
               1.4200000E+004,  8.2221987E+002
               1.4300000E+004,  8.2219966E+002
               1.4400000E+004,  8.2217966E+002
               1.4500000E+004,  8.2215986E+002
               1.4600000E+004,  8.2214027E+002
               1.4700000E+004,  8.2212086E+002
               1.4800000E+004,  8.2210165E+002
               1.4900000E+004,  8.2208262E+002
               1.5000000E+004,  8.2206375E+002'
  [../]
[]

[BCs]
  [./pellet_outer_temp]
    type = FunctionPresetBC
    variable = temp
    boundary = 0
    function = temp_bc
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
    value = 0.0
  [../]
  [./fixy2]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./fixz3]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]


#  [./temp0]
#    type = DirichletBC
#    variable = temp
#    boundary = 0
#    value = 0.0
#  [../]
#  [./temp1]
#    type = DirichletBC
#    variable = temp
#    boundary = 1
#    value = 1.0
#  [../]
#  [./fixx0]
#    type = DirichletBC
#    variable = disp_x
#    boundary = 0
#    value = 0.0
#  [../]
#  [./fixy0]
#    type = DirichletBC
#    variable = disp_y
#    boundary = 0
#    value = 0.0
#  [../]
#  [./fixz0]
#    type = DirichletBC
#    variable = disp_z
#    boundary = 0
#    value = 0.0
#  [../]
#  [./fixx1]
#    type = DirichletBC
#    variable = disp_x
#    boundary = 1
#    value = 0.0
#  [../]
#  [./fixy1]
#    type = DirichletBC
#    variable = disp_y
#    boundary = 1
#    value = 0.0
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
  [./bond_stretch]
    type = MaterialRealAux
    block = 0
    property = bond_stretch
    variable = bond_stretch
  [../]
  [./critical_stretch]
    type = MaterialRealAux
    block = 0
    property = critical_stretch
    variable = critical_stretch
  [../]
[]

[Executioner]
  type = Transient
  dt = 100.0
  solve_type = NEWTON #PJFNK
  l_max_its = 10000
  l_tol = 1e-06
  nl_max_its = 2000
#  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-6
  end_time = 1.0e4
  line_search = basic
#  petsc_options = '-snes_view -snes_check_jacobian -snes_check_jacobian_view'
  petsc_options_iname = '-pc-type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
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
  [./heatconduction]
    type = HeatConduction
    block = 0
    variable = temp
#    save_in = nodal_power
  [../]
  [./heatsource]
    type = HeatSourcePD
    block = 0
    variable = temp
    function = power_bc
    PowerDensity = 100.0
    save_in = nodal_power
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
    PDdim = 2                        # Dimension of Peridynamic model
    youngs_modulus = 200000
    poissons_ratio = 0.345
    MeshSpacing = 0.390476
#    MeshSpacing = 0.08118812               # MeshSpacing = (xmax - xmin) / nx
    ThicknessPerLayer = 1.0          # For 2D case, ThicknessPerLayer needs a value; For 3D case, not neccessary.
    CriticalStretch = 0.0005         # Expectation of the Gaussian Distribution
    StandardDeviation = 0.0001       # Standard Deviation of the Gaussian Distribution
    reference_temp = 300.0
    thermal_expansion = 0.00001
    thermal_conductivity = 5.0
  [../]
[]

[Postprocessors]
  [./sum_react_z_0]
    type = NodalSum
    variable = react_z
    boundary = 0
  [../]
  [./sum_react_z_1]
    type = NodalSum
    variable = react_z
    boundary = 1
  [../]
  [./sum_power_0]
    type = NodalSum
    variable = nodal_power
    boundary = 0
  [../]
  [./sum_power_1]
    type = NodalSum
    variable = nodal_power
    boundary = 1
  [../]
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
