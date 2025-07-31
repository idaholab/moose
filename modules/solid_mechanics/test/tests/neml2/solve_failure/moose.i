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
        incremental = true
        new_system = true
        add_variables = true
        volumetric_locking_correction = true
      []
    []
  []
[]

[AuxVariables]
  [temp]
    initial_condition = 1200
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
    function = 'xdisp_func'
    # Setting this to true is discouraged as it could plasticity solve more difficult.
    # We do it here simply to make the solve to fail to test the error handling.
    preset = true
  []
[]

[NEML2]
  input = 'neml2.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'
    moose_input_types = 'MATERIAL     MATERIAL POSTPROCESSOR POSTPROCESSOR MATERIAL              MATERIAL                  MATERIAL'
    moose_inputs = '     neml2_strain neml2_strain time          time          neml2_stress        equivalent_plastic_strain back_stress'
    neml2_inputs = '     forces/E     old_forces/E forces/t      old_forces/t  old_state/S old_state/internal/ep     old_state/internal/X'
    moose_output_types = 'MATERIAL     MATERIAL                  MATERIAL'
    moose_outputs = '     neml2_stress    equivalent_plastic_strain back_stress'
    neml2_outputs = '     state/S      state/internal/ep         state/internal/X'
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

[Functions]
  [xdisp_func]
    type = PiecewiseLinear
    x = '0 0.005 0.01 0.015 0.02'
    y = '0 0.005 0 -0.005 0'
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = 'none'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 1e-3
  dtmin = 1e-5
  end_time = 0.006
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  nl_max_its = 12
  line_search = none
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

