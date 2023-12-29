[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [igafile]
    type = FileMeshGenerator
    file = cframe_iga_coarse.e
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
  [disp_z]
    order = SECOND
    family = RATIONAL_BERNSTEIN
  []
[]

[Kernels]
  [TensorMechanics]
#Stress divergence kernels
    displacements = 'disp_x disp_y disp_z'
   []
[]

[AuxVariables]
    [von_mises]
        #Dependent variable used to visualize the von Mises stress
       order = SECOND
       family = MONOMIAL
    []
    [Max_Princ]
       #Dependent variable used to visualize the Hoop stress
       order = SECOND
       family = MONOMIAL
    []
    [stress_xx]
        order = SECOND
        family = MONOMIAL
    []
    [stress_yy]
        order = SECOND
        family = MONOMIAL
    []
    [stress_zz]
        order = SECOND
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
      boundary = '3'
      factor = 2000 # psi
    []
  []
  [anchor_x]
    #Anchors the bottom and sides against deformation in the x-direction
    type = DirichletBC
    variable = disp_x
    boundary = '2'
    value = 0.0
  []
  [anchor_y]
    #Anchors the bottom and sides against deformation in the y-direction
    type = DirichletBC
    variable = disp_y
    boundary = '2'
    value = 0.0
  []
  [anchor_z]
    #Anchors the bottom and sides against deformation in the z-direction
    type = DirichletBC
    variable = disp_z
    boundary = '2'
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor_AL]
    #Creates the elasticity tensor using concrete parameters
    youngs_modulus = 24e6 #psi
    poissons_ratio = 0.33
    type = ComputeIsotropicElasticityTensor
  []
  [strain]
    #Computes the strain, assuming small strains
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    #Computes the stress, using linear elasticity
    type = ComputeLinearElasticStress
  []
  [density_AL]
    #Defines the density of steel
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 6.99e-4 # lbm/in^3
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
  [max_principal_stress]
    type = PointValue
    point = '0.000000 -1.500000 -4.3'
    variable = Max_Princ
    use_displaced_mesh = false
  []
  [maxPrincStress]
    type = ElementExtremeValue
    variable = Max_Princ
  []
[]

[Executioner]
  # We solve a steady state problem using Newton's iteration
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-9
  l_max_its = 300
  l_tol = 1e-4
  nl_max_its = 30
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
[]

[Outputs]
  vtk = true
[]
