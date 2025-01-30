# NEML2 file in MPA
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
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
  [pressure_x]
    type = Pressure
    variable = disp_x
    boundary = right
    function = pressure_fcn
  []
[]

[Functions]
  [pressure_fcn]
    type = ParsedFunction
    expression = 200 #MPa
  []
[]
[Materials]
  [init_dd]
    type = GenericConstantMaterial
    prop_names = 'T init_cell_dd init_wall_dd init_envFac'
    prop_values = '750 1e11 8e12 2e15'
  []
[]

[NEML2]
  input = 'models/laromance_matl_radial_return.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    moose_input_types = 'MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL POSTPROCESSOR POSTPROCESSOR'
    moose_inputs =      'T        neml2_strain inelastic_strain eff_inelastic_strain cell_dd           wall_dd           init_envFac    time     time'
    neml2_inputs =      'forces/T forces/E     old_state/Ep     old_state/ep         old_state/cell_dd old_state/wall_dd forces/env_fac forces/t old_forces/t'

    moose_output_types = 'Material MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL MATERIAL'
    moose_outputs =      'neml2_stress inelastic_strain eff_inelastic_strain eff_inelastic_strain_rate vonmises_stress cell_rate       wall_rate       cell_dd       wall_dd'
    neml2_outputs =      'state/S      state/Ep         state/ep             state/ep_rate             state/s         state/cell_rate state/wall_rate state/cell_dd state/wall_dd'

    initialize_outputs =       'wall_dd      cell_dd      init_envFac'
    initialize_output_values = 'init_wall_dd init_cell_dd init_envFac'

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

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  nl_rel_tol = 1e-5
  dt = 50
  dtmin = 50
  num_steps = 10
  residual_and_jacobian_together = true
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [eff_inelastic_strain]
    type = ElementAverageMaterialProperty
    mat_prop = eff_inelastic_strain
  []
  [eff_inelastic_strain_rate]
    type = ElementAverageMaterialProperty
    mat_prop = eff_inelastic_strain_rate
  []
  [rhom_rate]
    type = ElementAverageMaterialProperty
    mat_prop = cell_rate
  []
  [rhoi_rate]
    type = ElementAverageMaterialProperty
    mat_prop = wall_rate
  []
  [rhom_dd]
    type = ElementAverageMaterialProperty
    mat_prop = cell_dd
  []
  [rhoi_dd]
    type = ElementAverageMaterialProperty
    mat_prop = wall_dd
  []
  [vm_stress]
    type = ElementAverageMaterialProperty
    mat_prop = vonmises_stress
  []
[]

[Outputs]
  csv = true
[]
