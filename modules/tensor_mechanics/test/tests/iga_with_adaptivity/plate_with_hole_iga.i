[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [igafile]
    type = FileMeshGenerator
    file = plate_with_hole_iga_in.e
    clear_spline_nodes = true
  []
[]

[Variables]
  [disp_x]
    order = SECOND
    family = RATIONAL_BERNSTEIN
  []
  [disp_y]
    order = SECOND
    family = RATIONAL_BERNSTEIN
  []
[]

[Kernels]
  [TensorMechanics]
    #Stress divergence kernels
    displacements = 'disp_x disp_y'
    planar_formulation = GENERALIZED_PLANE_STRAIN
   []
[]


[AuxVariables]
    [von_mises]
        #Dependent variable used to visualize the von Mises stress
        order = FIRST
        family = MONOMIAL
    []
    [Max_Princ]
        #Dependent variable used to visualize the Hoop stress
        order = FIRST
        family = MONOMIAL
    []
    [stress_pp]
        order = FIRST
        family = MONOMIAL
    []
    [stress_xx]
         order = FIRST
        family = MONOMIAL
    []
    [stress_yy]
         order = FIRST
        family = MONOMIAL
    []
    [stress_zz]
        order = FIRST
        family = MONOMIAL
    []
[]

[AuxKernels]
  [von_mises_kernel]
    #Calculates the von mises stress and assigns it to von_mises
    type = RankTwoScalarAux
    variable = von_mises
    rank_two_tensor = stress
    scalar_type = VonMisesStress
  []
  [MaxPrin]
    type = RankTwoScalarAux
    variable = Max_Princ
    rank_two_tensor = stress
    scalar_type = MaxPrincipal
  []
  [stress_xx]
    type = RankTwoAux
    index_i = 0
    index_j = 0
    rank_two_tensor = stress
    variable = stress_xx
  []
    [stress_yy]
    type = RankTwoAux
    index_i = 1
    index_j = 1
    rank_two_tensor = stress
    variable = stress_yy
  []
  [stress_zz]
    type = RankTwoAux
    index_i = 2
    index_j = 2
    rank_two_tensor = stress
    variable = stress_zz
  []
[]

[BCs]
  [Pressure]
    [load]
      #Applies the pressure
      boundary = '4'
      factor = -1 # Pa
    []
  []
  [anchor_x]
    #Anchors the bottom and sides against deformation in the x-direction
    type = DirichletBC
    variable = disp_x
    boundary = '3'
    value = 0.0
  []
  [anchor_y]
    #Anchors the bottom and sides against deformation in the y-direction
    type = DirichletBC
    variable = disp_y
    boundary = '2'
    value = 0.0
  []
[]

[Materials]
  active = 'density_AL stress strain elasticity_tensor_AL'
  [elasticity_tensor_AL]
    #Creates the elasticity tensor using concrete parameters
    youngs_modulus = 1000 #Pa
    poissons_ratio = 0.33
    type = ComputeIsotropicElasticityTensor
  []
  [strain]
    #Computes the strain, assuming small strains
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
  []
  [stress]
    #Computes the stress, using linear elasticity
    type = ComputeLinearElasticStress
  []
  [density_AL]
    #Defines the density of steel
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2710 # kg/m^3
  []
[]

[Preconditioning]
  [SMP]
    #Creates the entire Jacobian, for the Newton solve
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [maxPrincStress]
    type = ElementExtremeValue # NodalExtremeValue
    variable = Max_Princ
  []
  [maxStressProbe]
    type = PointValue
    point = '0 5 0'
    variable = Max_Princ
  []
  [numDOF]
    type = NumDOFs
  []
[]

[Executioner]
  #We solve a steady state problem using Newton's iteration
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-9
  l_max_its = 60
  l_tol = 1e-4
  nl_max_its = 20
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 61'
[]

[Outputs]
  [out]
    type = VTK
    execute_on = timestep_end
  []
[]
