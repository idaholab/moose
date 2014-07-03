#
# Test the non-split parsed function free enery Cahn-Hilliard Bulk kernel
# The free energy used here has teh same functional form as the CHPoly kernel
# If everything works, the output of this test should replicate the output
# of marmot/tests/chpoly_test/CHPoly_test.i (exodiff match)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4

  uniform_refine = 1
[]

[Variables]
  [./cv]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 6.0
      invalue = 1.0
      outvalue = 0.1
      int_width = 3.0
    [../]
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = cv
  [../]

  [./CHSolid]
    type = CHParsed
    variable = cv
    mob_name = M

    # Either define constants here...
    constant_names  = 'barr_height  cv_eq'
    constant_values = '0.1         1.0e-2'

    # ...or use material properties declared here.
    #material_property_names = 'barr_height cv_eq'

    # Equivalent to CHPoly with order=4
    function = 16*barr_height*(cv-cv_eq)^2*(1-cv_eq-cv)^2
  [../]

  [./CHInterface]
    type = CHInterface
    variable = cv
    mob_name = M
    grad_mob_name = grad_M
    kappa_name = kappa_c
  [../]
[]

[Materials]
  [./consts]
    type = PFMobility
    block = 0
    kappa = 0.1
    mob = 1e-3
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 2
  dt = 1.0

  [./Adaptivity]
    initial_adaptivity = 1
    error_estimator = LaplacianErrorEstimator
    refine_fraction = 0.8
    coarsen_fraction = 0.05
    max_h_level = 2
  [../]
[]

[Outputs]
  file_base = out
  output_initial = true
  interval = 1

  [./console]
    type = Console
    perf_log = true
  [../]

  [./OverSampling]
    type = Exodus
    refinements = 3
    output_initial = true
    oversample = true
    append_oversample = true
  [../]
[]
