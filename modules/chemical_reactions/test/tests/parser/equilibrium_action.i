# Test AqueousEquilibriumReactions parser

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./a]
  [../]
  [./b]
  [../]
[]

[AuxVariables]
  [./pressure]
  [../]
[]

[ICs]
  [./a]
    type = BoundingBoxIC
    variable = a
    x1 = 0.0
    y1 = 0.0
    x2 = 1.0e-10
    y2 = 1
    inside = 1.0e-2
    outside = 1.0e-10
  [../]
  [./b]
    type = BoundingBoxIC
    variable = b
    x1 = 0.0
    y1 = 0.0
    x2 = 1.0e-10
    y2 = 1
    inside = 1.0e-2
    outside = 1.0e-10
  [../]
  [./pressure]
    type = FunctionIC
    variable = pressure
    function = 2-x
  [../]
[]

[ReactionNetwork]
  [./AqueousEquilibriumReactions]
    primary_species = 'a b'
    reactions = '2a = pa2 2,
                 (1.0)a + (1.0)b = pab -2'
    secondary_species = 'pa2 pab'
    pressure = pressure
  [../]
[]

[Kernels]
  [./a_ie]
    type = PrimaryTimeDerivative
    variable = a
  [../]
  [./a_diff]
    type = PrimaryDiffusion
    variable = a
  [../]
  [./a_conv]
    type = PrimaryConvection
    variable = a
    p = pressure
  [../]
  [./b_ie]
    type = PrimaryTimeDerivative
    variable = b
  [../]
  [./b_diff]
    type = PrimaryDiffusion
    variable = b
  [../]
  [./b_conv]
    type = PrimaryConvection
    variable = b
    p = pressure
  [../]
[]

[BCs]
  [./a_left]
    type = DirichletBC
    variable = a
    boundary = left
    value = 1.0e-2
  [../]
  [./a_right]
    type = ChemicalOutFlowBC
    variable = a
    boundary = right
  [../]
  [./b_left]
    type = DirichletBC
    variable = b
    boundary = left
    value = 1.0e-2
  [../]
  [./b_right]
    type = ChemicalOutFlowBC
    variable = b
    boundary = right
  [../]
[]

[Materials]
  [./porous]
    type = GenericConstantMaterial
    prop_names = 'diffusivity conductivity porosity'
    prop_values = '1e-4 1e-4 0.2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_abs_tol = 1e-12
  end_time = 10
  dt = 10
[]

[Outputs]
  file_base = equilibrium_out
  exodus = true
  perf_graph = true
  print_linear_residuals = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
