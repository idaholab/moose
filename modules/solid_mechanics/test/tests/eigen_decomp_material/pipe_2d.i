# units in MPa, meters
[GlobalParams]
  order = SECOND
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Mesh]
  coord_type = RZ
  [pipe]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.2799334 #12"-0.979"
    xmax = .3048 #12"
    ymin = 0
    ymax = 0.00497 #0.979"/5
    nx = 5
    ny = 1
    elem_type = quad9
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        add_variables = true
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = false
        # generate_output = "cauchy_stress_xx cauchy_stress_yy cauchy_stress_zz cauchy_stress_xy "
        #                   "cauchy_stress_xz cauchy_stress_yz strain_xx strain_yy strain_zz strain_xy "
        #                   "strain_xz strain_yz vonmises_cauchy_stress"
      []
    []
  []
[]

[Functions]
  [inner_pressure]
    type = ConstantFunction
    value = 3
  []
  [big_pipe]
    type = ParsedFunction
    expression = -(p*11.021*11.021)/(12*12-11.021*11.021)
    symbol_names = 'p'
    symbol_values = 3
  []
[]

[BCs]
  [fixBottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
    preset = true
  []
  [Pressure]
    [inside]
      boundary = left
      function = inner_pressure
    []
    [axial]
      boundary = top
      function = big_pipe
    []
  []
[]

[Constraints]
  [top]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    secondary = top #sideset on top boundary
    penalty = 1e+14
    formulation = penalty
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 168000
    poissons_ratio = 0.31
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [nonADeig_decomp]
    type = EigenDecompMaterial
    rank_two_tensor = cauchy_stress
    outputs = exodus
    output_properties = "max_eigen_vector mid_eigen_vector min_eigen_vector "
                        "max_eigen_valuemid_eigen_value min_eigen_value"
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_rel_tol = 5e-8
[]

[Postprocessors]
  [eigval_max]
    type = ElementAverageMaterialProperty
    mat_prop = max_eigen_value
  []
[]

[Outputs]
  exodus = on
  console = true
[]
