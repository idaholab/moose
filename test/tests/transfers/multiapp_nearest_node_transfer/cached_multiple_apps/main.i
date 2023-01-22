[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = 2
    dy = 2
    dz = 2
    ix = 1
    iy = 5
    iz = 5
  []
  [translate]
    type = TransformGenerator
    input = cmg
    transform = TRANSLATE
    vector_value = '-1 -1 -1'
  []
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [Temperature]
  []
  [Layered_Average]
  []
  [Layered_Average_elem]
    family = MONOMIAL
    order = CONSTANT
  []
  [Subapp_Temp]
  []
  [Subapp_Temp_elem]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [extra]
    type = ADDiffusion
    variable = dummy
  []
[]

[AuxKernels]
  [Location_Based]
    type = ParsedAux
    variable = Temperature
    expression = 'x+y+z'
    use_xyzt = true
  []
  [Layered_Average_User_Object]
    type = SpatialUserObjectAux
    variable = Layered_Average
    user_object = Tfuel_UO
  []
  [Layered_Average_User_Object_elem]
    type = SpatialUserObjectAux
    variable = Layered_Average_elem
    user_object = Tfuel_UO
  []
[]

[UserObjects]
  [Tfuel_UO]
    type = NearestPointLayeredAverage
    variable = Temperature
    direction = x
    num_layers = 1
    points_file = 'locations.txt'
    execute_on = 'initial timestep_end'
  []
[]

[MultiApps]
  [TF_sub]
    type = FullSolveMultiApp
    positions_file = 'locations.txt'
    input_files = 'child.i'
    execute_on = 'TIMESTEP_END'
  []
[]

[GlobalParams]
  bbox_factor = 2
[]

[Transfers]
  [to_sub_layers]
    type = MultiAppNearestNodeTransfer
    to_multi_app = TF_sub
    source_variable = Layered_Average
    variable = Temperature
    fixed_meshes = True
  []
  [to_sub_layers_elem]
    type = MultiAppNearestNodeTransfer
    to_multi_app = TF_sub
    source_variable = Layered_Average_elem
    variable = Temperature_elem
    fixed_meshes = True
  []

  [from_sub_recover_layers]
    type = MultiAppNearestNodeTransfer
    from_multi_app = TF_sub
    source_variable = Temperature
    variable = Subapp_Temp
    fixed_meshes = True
  []
  [from_sub_recover_layers_elem]
    type = MultiAppNearestNodeTransfer
    from_multi_app = TF_sub
    source_variable = Temperature_elem
    variable = Subapp_Temp_elem
    fixed_meshes = True
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  # First step does not use Transfers caching
  # Second step does
  num_steps = 2
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'dummy Temperature Layered_Average Layered_Average_elem'
  []
[]
