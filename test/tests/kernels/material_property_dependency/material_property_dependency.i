[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  [u_ic]
    type = ConstantIC
    variable = u
    value = 0.5
  []
  [v_ic]
    type = ConstantIC
    variable = v
    value = 2.0
  []
[]

[Kernels]
  [u_hidden_mat_dep]
    type = MaterialPropertyDependencyKernel
    variable = u
    prop_name = hidden_prop
    dprop_dvar = 1
  []
  [v_reaction]
    type = Reaction
    variable = v
  []
[]

[Materials]
  [hidden_dependency]
    type = VarCouplingMaterial
    var = v
    use_tag = false
    base = 0
    coef = 1
    coupled_prop_name = hidden_prop
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = false
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
[]
