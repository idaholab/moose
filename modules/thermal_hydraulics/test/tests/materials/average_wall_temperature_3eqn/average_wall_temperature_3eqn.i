# Tests the average wall temperature aux for 1-phase flow. With the following
# inputs, the value should be equal to 1.25:
#
#   i   h_wall   T_wall   P_hf
#   --------------------------
#   1   10       26/10    1
#   2   6        1/2      3
#
#   T_fluid = 1/4
#
# With these values,
#   P_tot = 1 + 3 = 4
#   h_wall_avg = (1 * 10 + 3 * 6) / 4 = 28 / 4 = 7
#   denominator = P_tot * h_wall_avg = 4 * 7 = 28
#   numerator = 10 * (26/10 - 1/4) * 1 + 6 * (1/2 - 1/4) * 3 = 28
#   T_wall_avg = T_fluid + numerator / denominator = 1/4 + 1
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
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
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [Hw_avg]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_wall_avg]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_wall1]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_wall2]
    family = MONOMIAL
    order = CONSTANT
  []
  [P_hf1]
    family = MONOMIAL
    order = CONSTANT
  []
  [P_hf2]
    family = MONOMIAL
    order = CONSTANT
  []
  [P_hf_total]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_fluid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T_wall_avg_auxkernel]
    type = MaterialRealAux
    variable = T_wall_avg
    property = T_wall
  []
  [T_wall1_auxkernel]
    type = ConstantAux
    variable = T_wall1
    value = 2.6
  []
  [T_wall2_auxkernel]
    type = ConstantAux
    variable = T_wall2
    value = 0.5
  []
  [P_hf_total_auxkernel]
    type = SumAux
    variable = P_hf_total
    values = 'P_hf1 P_hf2'
  []
  [P_hf1_auxkernel]
    type = ConstantAux
    variable = P_hf1
    value = 1
  []
  [P_hf2_auxkernel]
    type = ConstantAux
    variable = P_hf2
    value = 3
  []
  [T_fluid_auxkernel]
    type = ConstantAux
    variable = T_fluid
    value = 0.25
  []
[]

[Materials]
  [const_materials]
    type = GenericConstantMaterial
    prop_names = 'Hw1 Hw2'
    prop_values = '10 6'
  []

  [T_wall_avg_material]
    type = AverageWallTemperature3EqnMaterial
    T_wall_sources = 'T_wall1 T_wall2'
    Hw_sources = 'Hw1 Hw2'
    P_hf_sources = 'P_hf1 P_hf2'
    T_fluid = T_fluid
    P_hf_total = P_hf_total
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [T_wall_avg_pp]
    type = ElementalVariableValue
    elementid = 0
    variable = T_wall_avg
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
