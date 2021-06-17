[Problem]
  solve = false
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [cut]
    type = LevelSetCutUserObject
    level_set_var = phi
    negative_id = 1
    positive_id = 33
    heal_always = true
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
  [phi]
  []
  [cut_id]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = 'x-0.213-t'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [cut_id]
    type = CutSubdomainIDAux
    variable = cut_id
    cut = cut
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
    geometric_cut_userobject = cut
    outputs = 'exodus'
    output_properties = 'D'
  []
[]

[Executioner]
  type = Transient

  dt = 0.25
  num_steps = 2

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
[]
