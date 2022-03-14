[GlobalParams]
  nx = 3
  ny = 3
  n_cells = 3
  pitch = 1
  heated_length = 0.2
[]

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    rod_diameter = 0.5
    gap = 0.1
    spacer_z = '0'
    spacer_k = '0'
  []

  [fuel_pins]
    type = PinMeshGenerator
    input = sub_channel
  []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[AuxVariables]
  [P]
    block = sub_channel
  []
  [T]
    block = fuel_pins
  []
[]

[AuxKernels]
  [P_ak]
    type = ParsedAux
    variable = P
    function = 'pow(x + 1, 2) + pow(2*(y + 1), 2) - (50 * z)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [T_ak]
    type = ParsedAux
    variable = T
    function = 'pow(x + 0.5, 2) + pow(2*(y + 0.5), 2) + (50 * z)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []
[]

[MultiApps]
  [viz]
    type = TransientMultiApp
    input_files = 'quad_viz.i'
  []
[]

[Transfers]
  [P_transfer]
    type = MultiAppDetailedSolutionTransfer
    direction = to_multiapp
    multi_app = viz
    variable = 'P'
  []
  [T_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    direction = to_multiapp
    multi_app = viz
    variable = 'T'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
