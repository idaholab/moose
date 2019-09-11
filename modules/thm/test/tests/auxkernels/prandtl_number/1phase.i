# Use PrandtlNumberAux to compute Prandtl number

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[AuxVariables]
  [prandtl_no]
  []
  [v]
    initial_condition = 1e-3
  []
  [e]
    initial_condition = 1e5
  []
[]

[AuxKernels]
  [pr_aux]
    type = PrandtlNumberAux
    variable = prandtl_no
    v = v
    e = e
    fp = fp
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [prandtl_no]
    type = ElementalVariableValue
    variable = prandtl_no
    elementid = 0
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
