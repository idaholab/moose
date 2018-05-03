[GlobalParams]
  displacements = 'disp_x disp_y'
  scalar_out_of_plane_strain = scalar_strain_zz
  full_jacobian = true
[]

[Mesh]
  type = GeneratedMeshPD
  dim = 2
  nx = 4
  horizon_number = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules]
  [./Peridynamics]
    [./Mechanics]
      formulation = OrdinaryState
    [../]
    [./GeneralizedPlaneStrain]
      [./gps]
        formulation = OrdinaryState
        out_of_plane_stress_variable = stress_zz
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_zz

    poissons_ratio = 0.3
    youngs_modulus = 1e6

    quantity_type = Component
    index_i = 2
    index_j = 2
  [../]
[]

[Materials]
  [./elastic_tensor]
    type = SmallStrainConstantHorizonOSPD
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_type'
    petsc_options_value = 'bcgs bjacobi test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
[]
