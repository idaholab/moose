[Problem]
  solve = false
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [cut1]
    type = LevelSetCutUserObject
    level_set_var = phi1
    negative_id = 1
    positive_id = 33
    execute_on = NONE
  []
  [cut2]
    type = LevelSetCutUserObject
    level_set_var = phi2
    negative_id = 5
    positive_id = 16
    execute_on = NONE
  []
  [combo]
    type = ComboCutUserObject
    geometric_cut_userobjects = 'cut1 cut2'
    cut_subdomain_combinations = '1 5;
                                  1 16;
                                  33 5;
                                  33 16'
    cut_subdomains = '1 3 5 7'
  []
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[AuxVariables]
  [phi1]
    [InitialCondition]
      type = FunctionIC
      function = 'x-0.213'
    []
  []
  [phi2]
    [InitialCondition]
      type = FunctionIC
      function = 'x-0.728'
    []
  []
  [cut1_id]
    order = CONSTANT
    family = MONOMIAL
  []
  [cut2_id]
    order = CONSTANT
    family = MONOMIAL
  []
  [combo_id]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [cut1_id]
    type = CutSubdomainIDAux
    variable = cut1_id
    cut = cut1
  []
  [cut2_id]
    type = CutSubdomainIDAux
    variable = cut2_id
    cut = cut2
  []
  [combo_id]
    type = CutSubdomainIDAux
    variable = combo_id
    cut = combo
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
  [diffusivity_C]
    type = GenericConstantMaterial
    prop_names = 'C_D'
    prop_values = '12'
  []
  [diffusivity_D]
    type = GenericConstantMaterial
    prop_names = 'D_D'
    prop_values = '9'
  []
  [diff_combined]
    type = XFEMCutSwitchingMaterialReal
    cut_subdomain_ids = '1 3 5 7'
    base_names = 'A B C D'
    prop_name = D
    geometric_cut_userobject = combo
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
