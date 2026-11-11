[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
[]

[NEML2]
  eager = true
  input = 'models/custom_model.i'
  load = 'models/test_models.py'
  inputs = 'A B'
  input_types = 'VARIABLE MATERIAL'
  [volume]
    model = model
    device = cpu
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [A]
  []
[]

[ICs]
  [A]
    type = ConstantIC
    variable = A
    value = 1
  []
[]

[Materials]
  [B]
    type = GenericConstantMaterial
    prop_names = B
    prop_values = 2
  []
  [make_sum_stateful]
    type = CoupledMaterial
    mat_prop = unused
    coupled_mat_prop = sum
    use_old_prop = true
  []
[]

[UserObjects]
  [internal_side]
    type = InsideUserObject
    variable = A
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
