## This example is documented on YouTube at:
## https://www.youtube.com/watch?v=L9plLYkAbvQ
## 
## Additional files (e.g. the CAD model, results)
## can be downloaded freely from Zenodo at:
## https://doi.org/10.5281/zenodo.3886965

[Mesh]
  displacements = 'disp_x disp_y disp_z' #Define displacements for deformed mesh
  type = FileMesh #Read in mesh from file
  file = piston_coarse.e
[]

# This is where mesh adaptivity magic happens:
[./Adaptivity]
  steps = 1
  #initial_steps = 2
  max_h_level = 3
  cycles_per_step = 1
  initial_marker = uniform
  marker = errorFraction
  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = refine
    [../]
    
    [./errorFraction]
      type = ErrorFractionMarker
      coarsen = 0.5
      indicator = gradientJump
      refine = 0.5
    [] 
 [../]

  [./Indicators]
    [./gradientJump]
      type = GradientJumpIndicator
      variable = disp_y
    []
  []
[../]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    #Stress divergence kernels
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[AuxVariables]
  [./von_mises]
    #Dependent variable used to visualize the Von Mises stress
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./von_mises_kernel]
    #Calculates the von mises stress and assigns it to von_mises
    type = RankTwoScalarAux
    variable = von_mises
    rank_two_tensor = stress
    execute_on = timestep_end
    scalar_type = VonMisesStress
  [../]
[]

[Functions]
  [./rampLinear]
    type = ParsedFunction
    value = t*550e5
  []
[]

[BCs]
  [./Pressure]
    [./load]
      #Applies the pressure
      boundary = load_surf
      function = rampLinear
      displacements = 'disp_x disp_y disp_z'
    [../]
  [../]
  [./symmetry_x]
    #Applies symmetry on the xmin faces
    type = DirichletBC
    variable = disp_x
    boundary = 'xmin'
    value = 0.0
  [../]
  [./hold_y]
    #Anchors the bottom against deformation in the y-direction
    type = DirichletBC
    variable = disp_y
    boundary = 'ymin'
    value = 0.0
  [../]
  [./symmetry_z]
    #Applies symmetry on the zmin faces
    type = DirichletBC
    variable = disp_z
    boundary = 'zmin'
    value = 0.0
  [../]
[]

[Materials]
  active = 'density_steel stress strain elasticity_tensor_steel'
  [./elasticity_tensor_steel]
    #Creates the elasticity tensor using steel parameters
    youngs_modulus = 210e9 #Pa
    poissons_ratio = 0.3
    type = ComputeIsotropicElasticityTensor
    block = 1
  [../]
  [./strain]
    #Computes the strain, assuming small strains
    type = ComputeFiniteStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    #Computes the stress, using linear elasticity
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]
  [./density_steel]
    #Defines the density of steel
    type = GenericConstantMaterial
    block = 1
    prop_names = density
    prop_values = 7850 # kg/m^3
  [../]
[]

[Preconditioning]
  [./SMP]
    #Creates the entire Jacobian, for the Newton solve
    type = SMP
    full = true
  [../]
[]

[Executioner]
  #We solve a steady state problem using Newton's iteration
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
