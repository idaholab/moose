N = 2

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = ${N}
    ny = ${N}
    nz = ${N}
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        new_system = true
        add_variables = true
        formulation = TOTAL
        volumetric_locking_correction = true
      []
    []
  []
[]

[NEML2]
  input = 'elasticity_neml2.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'MATERIAL'
    moose_inputs = 'neml2_strain'
    neml2_inputs = 'forces/E'

    moose_output_types = 'MATERIAL'
    moose_outputs = 'neml2_stress'
    neml2_outputs = 'state/S'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = 'neml2_jacobian'
    neml2_derivatives = 'state/S forces/E'
  []
[]

[Materials]
  [convert_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'mechanical_strain'
    to = 'neml2_strain'
  []
  [stress]
    type = ComputeLagrangianObjectiveCustomSymmetricStress
    custom_small_stress = 'neml2_stress'
    custom_small_jacobian = 'neml2_jacobian'
  []
[]

[BCs]
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [zfix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [xdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = t
    preset = false
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 1e-3
  dtmin = 1e-3
  num_steps = 5
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
