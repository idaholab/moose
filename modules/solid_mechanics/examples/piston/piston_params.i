## This example is documented on YouTube at:
## https://www.youtube.com/watch?v=L9plLYkAbvQ
##
## Additional files (e.g. the CAD model, results)
## can be downloaded freely from Zenodo at:
## https://doi.org/10.5281/zenodo.3886965

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  # Read in mesh from file
  type = FileMesh
  file = piston_coarse.e
[]

# This is where mesh adaptivity magic happens
[Adaptivity]
  steps = 1
  max_h_level = 3
  cycles_per_step = 1
  initial_marker = uniform
  marker = errorFraction
  [Markers]
    [uniform]
      type = UniformMarker
      mark = refine
    []
    [errorFraction]
      type = ErrorFractionMarker
      coarsen = 0.5
      indicator = gradientJump
      refine = 0.5
    []
  []

  [Indicators]
    [gradientJump]
      type = GradientJumpIndicator
      variable = disp_y
    []
  []
[]

[Modules/TensorMechanics/Master]
  # Parameters that apply to all subblocks are specified at this level.
  # They can be overwritten in the subblocks.
  add_variables = true
  incremental = false
  strain = SMALL
  generate_output = 'vonmises_stress'
  [block]
    block = 1
  []
[]

[BCs]
  [Pressure]
    [load]
      # Applies the pressure
      boundary = load_surf
      function = 't*550e5'
    []
  []
  [symmetry_x]
    # Applies symmetry on the xmin faces
    type = DirichletBC
    variable = disp_x
    boundary = 'xmin'
    value = 0.0
  []
  [hold_y]
    # Anchors the bottom against deformation in the y-direction
    type = DirichletBC
    variable = disp_y
    boundary = 'ymin'
    value = 0.0
  []
  [symmetry_z]
    # Applies symmetry on the zmin faces
    type = DirichletBC
    variable = disp_z
    boundary = 'zmin'
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor_steel]
    # Creates the elasticity tensor using steel parameters
    youngs_modulus = 210e9 #Pa
    poissons_ratio = 0.3
    type = ComputeIsotropicElasticityTensor
    block = 1
  []
  [stress]
    # Computes the stress, using linear elasticity
    type = ComputeLinearElasticStress
    block = 1
  []
[]

[Preconditioning]
  [SMP]
    # Creates the entire Jacobian, for the Newton solve
    type = SMP
    full = true
  []
[]

[Executioner]
  # We solve a steady state problem using Newton's iteration
  type = Transient
  solve_type = NEWTON
  nl_rel_tol = 1e-9
  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 10
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
