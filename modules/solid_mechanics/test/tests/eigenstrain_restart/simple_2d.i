[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [pipe]
    type = GeneratedMeshGenerator
    nx = 8
    xmin = 10
    xmax = 200
    ny = 5
    ymin = 0
    ymax = 100
    dim = 2
  []
  coord_type = 'RZ'
  rz_coord_axis = y
  use_displaced_mesh = false
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
        incremental = true
        eigenstrain_names = 'dummy_eigenstrain'
        use_automatic_differentiation = true
        generate_output = 'plastic_strain_xx plastic_strain_yy plastic_strain_zz
                           plastic_strain_xy plastic_strain_xz plastic_strain_yz'
        material_output_order = 'FIRST FIRST FIRST
                                 FIRST FIRST FIRST'
      []
    []
  []
[]

[Functions]
  [isohard_alloy600]
    type = PiecewiseLinear
    x = '0 100'
    y = '214.2 215.2'
  []
  [eig_function]
    type = ParsedFunction
    expression = '0.0001*(x-100)^2*t'
  []
  [inner_pressure]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0.0 200 0'
  []
[]

[Materials]
  [radial_return_stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'isoplasticity'
    max_iterations = 1000
    relative_tolerance = 1e-08
    absolute_tolerance = 1e-11
    perform_finite_strain_rotations = false
  []
  [isoplasticity]
    type = ADIsotropicPlasticityStressUpdate
    yield_stress = 214.2
    hardening_function = isohard_alloy600
    max_inelastic_increment = 0.0001
    relative_tolerance = 1e-08
    absolute_tolerance = 1e-11
  []
  [elastic]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 183150
    poissons_ratio = 0.3
  []
  [dummy_eigen]
    type = ADGenericFunctionRankTwoTensor
    tensor_name = dummy_eigenstrain
    tensor_functions = 'eig_function 0            0
                        0            eig_function 0
                        0            0            eig_function'
  []
[]

[BCs]
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [inside]
    type = FunctionNeumannBC
    boundary = left
    variable = disp_x
    function = inner_pressure
  []
[]

[AuxVariables]
  [th_iso_aux]
    family = MONOMIAL
    order = FIRST
  []
  [eig_rr]
    family = MONOMIAL
    order = FIRST
  []
  [eig_yy]
    family = MONOMIAL
    order = FIRST
  []
  [eig_ry]
    family = MONOMIAL
    order = FIRST
  []
  [eig_tt]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [th_iso_aux_kernel]
    type = ADMaterialRankTwoTensorAux
    i = 0
    j = 0
    property = dummy_eigenstrain
    variable = th_iso_aux
    execute_on = TIMESTEP_END
  []
  [eig_rr_kernel]
    type = ParsedAux
    variable = eig_rr
    coupled_variables = 'plastic_strain_xx th_iso_aux'
    expression = 'plastic_strain_xx + th_iso_aux'
    execute_on = TIMESTEP_END
  []
  [eig_yy_kernel]
    type = ParsedAux
    variable = eig_yy
    coupled_variables = 'plastic_strain_yy th_iso_aux'
    expression = 'plastic_strain_yy + th_iso_aux'
    execute_on = TIMESTEP_END
  []
  [eig_ry_kernel]
    type = ParsedAux
    variable = eig_ry
    coupled_variables = 'plastic_strain_xy'
    expression = 'plastic_strain_xy'
    execute_on = TIMESTEP_END
  []
  [eig_tt_kernel]
    type = ParsedAux
    variable = eig_tt
    coupled_variables = 'plastic_strain_zz th_iso_aux'
    expression = 'plastic_strain_zz + th_iso_aux'
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-6
  nl_abs_tol = 5e-7
  dt = 1
  end_time = 2
[]

[Outputs]
  exodus = true
  [handoff]
    type = XDA
    file_base = simple_2d_out
    execute_on = 'Final'
  []
[]
