# Computes nodal void volume, when using adaptivity, and compares with the Postprocessor hand-calculated values
[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 2 2'
    dy = '1 4'
  []
[]

[Adaptivity]
  initial_marker = u_marker
  marker = u_marker
  max_h_level = 1
  [Markers]
    [u_marker]
      type = ValueRangeMarker
      variable = u
      invert = true
      lower_bound = 0.02
      upper_bound = 0.98
    []
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'if(x<2,0,1)'
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = u
  []
  [u]
    type = Diffusion
    variable = u
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  csv = true
[]

[UserObjects]
  [nodal_void_volume]
    type = NodalVoidVolume
    porosity = porosity
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
  [vol]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    variable = porosity
    function = 'if(x<4, 1, 2)'
  []
  [vol]
    type = NodalVoidVolumeAux
    variable = vol
    nodal_void_volume_uo = nodal_void_volume
  []
[]

[Postprocessors]
  [quarter]
    type = PointValue
    point = '0 0 0'
    variable = vol
  []
  [half]
    type = PointValue
    point = '1 0 0'
    variable = vol
  []
  [three_quarters]
    type = PointValue
    point = '2 0 0'
    variable = vol
  []
  [one_and_half_to_34s]
    type = PointValue
    point = '4 0 0'
    variable = vol
  []
  [one_to_14]
    type = PointValue
    point = '6 0 0'
    variable = vol
  []
  [one_and_quarter]
    type = PointValue
    point = '0 1 0'
    variable = vol
  []
  [two_and_half]
    type = PointValue
    point = '1 1 0'
    variable = vol
  []
  [three_and_three_quarters]
    type = PointValue
    point = '2 1 0'
    variable = vol
  []
  [seven_and_half_to_334]
    type = PointValue
    point = '4 1 0'
    variable = vol
  []
  [five_to_54]
    type = PointValue
    point = '6 1 0'
    variable = vol
  []
[]
