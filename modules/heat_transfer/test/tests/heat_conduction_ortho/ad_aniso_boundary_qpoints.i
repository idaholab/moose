# The specific heat is the x coordinate, so its average on the displaced right boundary is the
# boundary position 1 + disp_x = 1.1.
[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [disp_x]
    initial_condition = 0.1
  []
  [disp_y]
  []
[]

[Kernels]
  [diffusion]
    type = ADDiffusion
    variable = T
  []
[]

[Materials]
  [aniso]
    type = ADAnisoHeatConductionMaterial
    temperature = T
    thermal_conductivity = '1 0 0 0 1 0 0 0 1'
    specific_heat = x
  []
[]

[Postprocessors]
  [right_specific_heat]
    type = ADSideAverageMaterialProperty
    boundary = right
    property = specific_heat
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
