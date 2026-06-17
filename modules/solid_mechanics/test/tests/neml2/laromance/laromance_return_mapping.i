# NEML2 file in MPA
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
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
    expression = 'if(t<200,t,200)' #MPa
  []
[]

[Materials]
  [init_dd]
    type = GenericConstantMaterial
    prop_names = 'temperature init_cell_dd init_wall_dd env_factor'
    prop_values = '750 1e11 8e12 2e15'
  []
[]

[NEML2]
  input = 'models/laromance_matl_radial_return.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'
    initialize_outputs = 'wall_dislocation_density cell_dislocation_density'
    initialize_output_values = 'init_wall_dd init_cell_dd'
    derivatives = 'neml2_stress neml2_strain'
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
    custom_small_jacobian = 'dneml2_stress/dneml2_strain'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  dt = 50
  dtmin = 50
  num_steps = 10
  residual_and_jacobian_together = true
[]

[Postprocessors]
  [eff_inelastic_strain]
    type = ElementAverageMaterialProperty
    mat_prop = equivalent_plastic_strain
  []
  [rhom_dd]
    type = ElementAverageMaterialProperty
    mat_prop = cell_dislocation_density
  []
  [rhoi_dd]
    type = ElementAverageMaterialProperty
    mat_prop = wall_dislocation_density
  []
  [vm_stress]
    type = ElementAverageMaterialProperty
    mat_prop = von_mises_stress
  []
[]

[Outputs]
  csv = true
[]
