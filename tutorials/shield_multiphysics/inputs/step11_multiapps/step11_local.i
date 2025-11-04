[Mesh]
  # We make a 3D sphere, but really this could be done in 1D
  [sphere]
    type = SphereMeshGenerator
    radius = 10
    nr = 3
    n_smooth = 10
  []
  # Dimensions of each layer are not realistic
  [HDPE_inner]
    type = ParsedSubdomainMeshGenerator
    input = 'sphere'
    combinatorial_geometry = 'x*x + y*y + z*z < 2*2'
    block_id = 1
  []
  [boron_inner]
    type = ParsedSubdomainMeshGenerator
    input = 'HDPE_inner'
    combinatorial_geometry = '(x*x + y*y + z*z > 2*2) & (x*x + y*y + z*z < 3*3)'
    block_id = 2
  []
  [HDPE_mid]
    type = ParsedSubdomainMeshGenerator
    input = 'boron_inner'
    combinatorial_geometry = '(x*x + y*y + z*z > 3*3) & (x*x + y*y + z*z < 6*6)'
    block_id = 3
  []
  [boron_mid]
    type = ParsedSubdomainMeshGenerator
    input = 'HDPE_mid'
    combinatorial_geometry = '(x*x + y*y + z*z > 6*6) & (x*x + y*y + z*z < 7*7)'
    block_id = 4
  []
  [HDPE_outer]
    type = ParsedSubdomainMeshGenerator
    input = 'boron_mid'
    combinatorial_geometry = 'x*x + y*y + z*z > 7*7'
    block_id = 5
  []
  [rename]
    type = RenameBlockGenerator
    input = 'HDPE_outer'
    old_block = '1 2 3 4 5'
    new_block = 'HDPE_inner boron_inner HDPE_mid boron_mid HDPE_outer'
  []
  [rename_boundary]
    type = RenameBoundaryGenerator
    input = 'rename'
    old_boundary = '0'
    new_boundary = 'outer'
  []

  # length_unit = 0.01
  [scale]
    type = TransformGenerator
    input = rename_boundary
    transform = SCALE
    vector_value = '0.01 0.01 0.01'
  []
[]

[Variables]
  [T]
    initial_condition = 300
  []
[]

# Solve heat equation, with a source from boron absorption
[Kernels]
  [conduction]
    type = HeatConduction
    variable = T
  []
  [source]
    type = CoupledForce
    variable = T
    block = 'boron_inner boron_mid'
    v = flux
    # 2 is our arbitrary value for the group cross section
    coef = 2
  []
[]

[BCs]
  [outer]
    type = PostprocessorDirichletBC
    boundary = 'outer'
    variable = 'T'
    postprocessor = 'T_boundary'
  []
[]

[AuxVariables]
  # Received from the main solve
  [flux]
    initial_condition = 1e5
  []
[]

[Materials]
  [hdpe]
    type = HeatConductionMaterial
    block = 'HDPE_inner HDPE_mid HDPE_outer'
    # arbitrary
    thermal_conductivity = 10
  []
  [boron]
    type = HeatConductionMaterial
    block = 'boron_inner boron_mid'
    # arbitrary
    thermal_conductivity = 7
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  # Used for boundary condition, received from the main solve
  [T_boundary]
    type = Receiver
    default = 320
  []

  # Compute those then send to the main app
  [T_hdpe_out]
    type = ElementAverageValue
    variable = 'T'
    block = 'HDPE_outer'
  []
  [T_boron_mid]
    type = ElementAverageValue
    variable = 'T'
    block = 'boron_mid'
  []
  [T_hdpe_mid]
    type = ElementAverageValue
    variable = 'T'
    block = 'HDPE_mid'
  []
  [T_boron_inner]
    type = ElementAverageValue
    variable = 'T'
    block = 'boron_inner'
  []
  [T_hdpe_inner]
    type = ElementAverageValue
    variable = 'T'
    block = 'HDPE_inner'
  []
[]

[Outputs]
  exodus = true
[]
