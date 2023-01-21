[Mesh]
  type = MeshGeneratorMesh
  displacements = 'disp_x disp_y'
  [./left_gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 3
    xmin = -3
    xmax = 0
    ymin = -5
    ymax = 5
  [../]
  [./right_gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 3
    xmin = 3
    xmax = 6
    ymin = -5
    ymax = 5
  [../]

  [./left_and_right]
    type = MeshCollectionGenerator
    inputs = 'left_gen right_gen'
  [../]
  [./leftleft]
    type = SideSetsAroundSubdomainGenerator
    block = 0
    new_boundary = leftleft
    normal = '-1 0 0'
    input = left_and_right
  [../]
  [./leftright]
    type = SideSetsAroundSubdomainGenerator
    block = 0
    new_boundary = leftright
    normal = '1 0 0'
    input = leftleft
  [../]

  [./right]
    type = SubdomainBoundingBoxGenerator
    top_right = '6 5 0'
    bottom_left = '3 -5 0'
    block_id = 1
    input = leftright
  [../]

  [./rightleft]
    type = SideSetsAroundSubdomainGenerator
    block = 1
    new_boundary = rightleft
    normal = '-1 0 0'
    input = right
  [../]
  [./rightright]
    type = SideSetsAroundSubdomainGenerator
    block = 1
    new_boundary = rightright
    normal = '1 0 0'
    input = rightleft
  [../]
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./gap_conductance]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./disp_x]
    type = ParsedFunction
    expression = -3+t
  [../]
  [./left_temp]
    type = ParsedFunction
    expression = 1000+t
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
  [../]
[]

[AuxKernels]
  [./disp_x]
    type = FunctionAux
    block = 1
    variable = disp_x
    function = disp_x
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./gap_conductivity]
    type = MaterialRealAux
    boundary = leftright
    property = gap_conductance
    variable = gap_conductance
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = temp
    boundary = leftleft
    function = left_temp
  [../]
  [./right]
    type = DirichletBC
    variable = temp
    boundary = rightright
    value = 400
  [../]
[]

[ThermalContact]
  [./left_to_right]
    secondary = leftright
    quadrature = true
    primary = rightleft
    variable = temp
    min_gap = 1
    min_gap_order = 1
    emissivity_primary = 0
    emissivity_secondary = 0
    type = GapHeatTransfer
  [../]
[]

[Materials]
  [./hcm]
    type = HeatConductionMaterial
    block = '0 1'
    specific_heat = 1
    thermal_conductivity = 1
    use_displaced_mesh = true
  [../]
[]

[Postprocessors]
  [./gap_conductance]
    type = PointValue
    point = '0 0 0'
    variable = gap_conductance
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.25
  end_time = 3.0
  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
