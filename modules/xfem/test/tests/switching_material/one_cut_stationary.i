[Problem]
  solve = false
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [levelset_cut]
    type = LevelSetCutUserObject
    level_set_var = phi
    negative_id = 1
    positive_id = 33
  []
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[AuxVariables]
  [phi]
    [InitialCondition]
      type = FunctionIC
      function = 'x-0.213'
    []
  []
[]

[Materials]
  [diffusivity_A]
    type = GenericConstantMaterial
    prop_names = 'A_D'
    prop_values = '5'
  []
  [diffusivity_B]
    type = GenericConstantMaterial
    prop_names = 'B_D'
    prop_values = '0.5'
  []
  [diff_combined]
    type = XFEMCutSwitchingMaterialReal
    cut_subdomain_ids = '1 33'
    base_names = 'A B'
    prop_name = D
    geometric_cut_userobject = levelset_cut
    outputs = 'exodus'
    output_properties = 'D'
  []
[]

[Executioner]
  type = Transient

  num_steps = 1

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
[]
