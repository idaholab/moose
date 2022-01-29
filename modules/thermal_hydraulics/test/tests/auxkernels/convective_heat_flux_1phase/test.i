# The test computes convective heat flux for single phase flow according to:
#
#   q = Hw * (T_wall - T_fluid)
#
# where Hw = 2, T_wall = 310 and T_fluid = 300. Thus, q = 20.
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[AuxVariables]
  [q_wall]
    family = MONOMIAL
    order = CONSTANT
  []

  [T_wall]
  []
[]

[ICs]
  [T_wall_ic]
    type = ConstantIC
    variable = T_wall
    value = 310
  []
[]

[AuxKernels]
  [sound_speed_aux]
    type = ConvectiveHeatFlux1PhaseAux
    variable = q_wall
    Hw = Hw
    T_wall = T_wall
    T = T
  []
[]

[Materials]
  [mats]
    type = GenericConstantMaterial
    prop_names = 'T Hw'
    prop_values = '300 2'
  []
[]

[Postprocessors]
  [q_wall]
    type = ElementalVariableValue
    variable = q_wall
    elementid = 0
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bcs]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 1
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
  execute_on = TIMESTEP_END
[]
