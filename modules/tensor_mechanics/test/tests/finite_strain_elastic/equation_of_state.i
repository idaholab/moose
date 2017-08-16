# Isotropic compression test on one element
# for a linear elastic material with Murnaghan equation of state
# expected stress_zz is: P = (K0 / n) * ( 1.0 - J^{-n} )
# J is the Jacobian: J = detF = v / v0, F = deformation gradient
# v and v0 are the specific volumes: deformed and reference
# Austin et al. JOURNAL OF APPLIED PHYSICS 117, 185902 (2015)
#
# The stress component stress_zz is equal to the pressure
# P = (K0 / n) * ( 1.0 - J^{-n} ), K0 = 15.588, n = 6.6
# At final timestep, disp_x = disp_y = disp_z = -0.1
# J = 0.9^3 and stress_zz = -16.660

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    value = -0.001*t
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
[]

[BCs] # isotropic compression boundary conditions
  [./symmx]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmy]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmz]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tdisp_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = right
    function = tdisp
  [../]
  [./tdisp_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = tdisp
  [../]
  [./tdisp_z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = tdisp
  [../]
[]

[Materials]
  [./crysp]
    type = ComputeFiniteStrainElasticStressBirchMurnaghan
    # Bulk viscosity damping parameters:
    # Maheo et al. Mechanics Research Communications 38 (2011) 81 88
    C0 = 2.0
    C1 = 10.0
    # Birch-Murnaghan equation of state parameters:
    # Austin et al. JOURNAL OF APPLIED PHYSICS 117, 185902 (2015)
    n_Murnaghan = 6.6
    bulk_modulus_ref = 15.588
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./elasticity_tensor] # elasticity constant for HMX as in Barton et al. Modelling Simul. Mater. Sci. Eng. 17 (2009) 035003
    type = ComputeElasticityTensorCP
    C_ijkl = '9.6 10.1 0.0'
    fill_method = general_isotropic
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  end_time = 100.0
  dt = 1.0
[]

[Outputs]
  exodus = true
  interval = 10
[]
