[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    components = 2
  []
[]

[AuxVariables]
  [v]
    components = 4
    initial_condition = '1 2 3 4'
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [force]
    type = ArrayCoupledForceVar
    variable = u
    v_coef = 'v 1.5,2.5,3.5,4.5'
  []
[]

[BCs]
  [vac]
    type = ArrayVacuumBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
