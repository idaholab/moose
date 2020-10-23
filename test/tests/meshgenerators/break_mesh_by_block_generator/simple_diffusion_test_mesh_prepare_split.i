[Mesh]
  [msh]
    type = FileMeshGenerator
    #file = this the exodus or checkpoint file we want to use
    has_fake_neighbors = true
    # exodus_extra_element_integers = 'bmbb_element_id' this iwll be neeeded to reconstruct the broken mesh
    fake_neighbor_list_file_name = 'fake_neighbors_test_bmbb.csv'
  []
[]

[AuxVariables]
  [bmbb_element_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = u
    boundary = interface
    penalty = 1e6
  [../]
[]

[AuxKernels]
  [set_material_id]
    type = ElemExtraIDAux
    variable = bmbb_element_id
    extra_id_name = bmbb_element_id
  []
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [diff]
    type = AnisotropicDiffusion
    variable = u
    tensor_coeff = '2 0 0
                    0 4 0
        0 0 6'
  []
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[Functions]
  [./top_bc]
    type = ParsedFunction
    value = 'x'
  [../]
[]

[BCs]
  [./lower_left]
    type = DirichletBC
    variable = u
    boundary = 'bottom left'
    value = 1
  [../]

  [./top]
    type = FunctionNeumannBC
    variable = u
    boundary = top
    function = top_bc
  [../]

  [./right]
    type = NeumannBC
    variable = u
    boundary = right
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
