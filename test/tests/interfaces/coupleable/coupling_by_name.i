[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [v][]
[]

[AuxKernels]
  [v]
    type = ParsedAux
    variable = v
    use_xyzt = true
    expression = x
    execute_on = 'linear initial'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForceVar
    variable = u
    v_coef = 'v 2.0'
  []
[]

[BCs]
  [vac]
    type = VacuumBC
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
