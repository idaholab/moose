#
# Bridge linear elasticity example
#
# This example models a bridge using linear elasticity.
# It can be either steel or concrete.
# Gravity is applied
# A pressure of 0.5 MPa is also applied
#

[Mesh]
  displacements = 'disp_x disp_y disp_z' #Define displacements for deformed mesh
  type = FileMesh #Read in mesh from file
  file = bridge.e

  boundary_id = '1 2 3 4 5 6' #Assign names to boundaries to make things clearer
  boundary_name = 'top left right bottom1 bottom2 bottom3'
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

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
  [./gravity_y]
    #Gravity is applied to bridge
    type = Gravity
    variable = disp_y
    value = -9.81
  [../]
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

[BCs]
  [./Pressure]
    [./load]
      #Applies the pressure
      boundary = top
      factor = 5e5 # Pa
    [../]
  [../]
  [./anchor_x]
    #Anchors the bottom and sides against deformation in the x-direction
    type = DirichletBC
    variable = disp_x
    boundary = 'left right bottom1 bottom2 bottom3'
    value = 0.0
  [../]
  [./anchor_y]
    #Anchors the bottom and sides against deformation in the y-direction
    type = DirichletBC
    variable = disp_y
    boundary = 'left right bottom1 bottom2 bottom3'
    value = 0.0
  [../]
  [./anchor_z]
    #Anchors the bottom and sides against deformation in the z-direction
    type = DirichletBC
    variable = disp_z
    boundary = 'left right bottom1 bottom2 bottom3'
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
  [./elasticity_tensor_concrete]
    #Creates the elasticity tensor using concrete parameters
    youngs_modulus = 16.5e9 #Pa
    poissons_ratio = 0.2
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
  [./density_concrete]
    #Defines the density of concrete
    type = GenericConstantMaterial
    block = 1
    prop_names = density
    prop_values = 2400 # kg/m^3
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
  num_steps = 1
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
