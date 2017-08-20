# Unidirectional tensile test on one element
# for a material described by phase-field damage
# and temperature increase due to crack propagation.
#
# The positive component of the specific strain energy
# G0_pos is: G0_pos = lambda * strain_zz^2 / 2 + mu * strain_zz^2
# lambda and mu are the Lame parameters
#
# the phase-field damage c evolves as:
# dc/dt = (1 / visco) * (2 * (1 - c) * G0_pos / gc - c / l)
# visco is the viscosity parameters
# gc is the critical energy release rate for fracture
# l is the damage zone thickness
#
# The crack propagation heat rate is:
# crack_propagation_heat = 2 * (1 - c) * G0_pos * (dc/dt)
#
# Heat equation for the temperature T is:
# rho cp dT/dt = crack_propagation_heat
# rho is the density
# cp is the specific heat
#
# C. Miehe, L.M. Schanzel, H. Ulmer, Comput. Methods Appl. Mech. Engrg. 294 (2015) 449 - 485
# P. Chakraborty, Y. Zhang, M.R. Tonks, Multi-scale modeling of microstructure dependent
# inter-granular fracture in UO2 using a phase-field based method
# Idaho National Laboratory technical report
#
# strain_zz is the only non-zero strain component
# and goes from 0 to 0.01 (1%)
# Lame parameters: lambda = 1, mu = 1/2
# G0_pos = strain_zz^2
#
# phase-field model parameters:
# visco = 1
# gc = 2
# l = 1
# dc/dt = (1 - c) * strain_zz^2 - c
# for c = 0 => dc/dt = strain_zz^2
# for c = 0 => crack_propagation_heat = 2 * strain_zz^4
#
# at t = 0: strain_zz = 0
# at t = 1: strain_zz = 0.01
# therefore damage at t = 1 is (time integration):
# 0.5 * (strain_zz^2(t=0) + strain_zz^2(t=1)) = 5e-5
# and:
# crack_propagation_heat = 0.5 * (2 * strain_zz^4(t=0) + 2 * strain_zz^4(t=1)) = 1e-8
#
# heat equation parameters
# rho = 1
# cp = 1
# dT/dt = crack_propagation_heat

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./c]
  [../]
  [./b]
  [../]
  [./temperature]
  [../]
[]

[AuxVariables]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./G0_pos]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_propagation_heat]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    value = 0.01
  [../]
[]

[Kernels]
  [./pfbulk]
    type = SplitPFFractureBulkRate
    variable = c
    width = 1.0
    beta = b
    viscosity = 1.0
    gc = 'gc_prop'
    G0 = 'G0_pos'
    dG0_dstrain = 'dG0_pos_dstrain'
  [../]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./solid_x]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_x
    component = 0
    c = c
  [../]
  [./solid_y]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_y
    component = 1
    c = c
  [../]
  [./solid_z]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_z
    component = 2
    c = c
  [../]
  [./dcdt]
    type = TimeDerivative
    variable = c
  [../]
  [./pfintvar]
    type = Reaction
    variable = b
  [../]
  [./pfintcoupled]
    type = LaplacianSplit
    variable = b
    c = c
  [../]
  [./hct]
    type = HeatConductionTimeDerivative
    variable = temperature
    specific_heat = specific_heat
    density_name = density
  [../]
  [./crack_heat]
    type = CrackPropagationHeatEnergy
    variable = temperature
    c = c
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[AuxKernels]
  [./strain_zz]
    type = RankTwoAux
    variable = strain_zz
    rank_two_tensor = total_strain
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
  [./G0_pos]
    type = MaterialRealAux
    variable = G0_pos
    property = G0_pos
    execute_on = timestep_end
  [../]
  [./crack_propagation_heat]
    type = MaterialRealAux
    variable = crack_propagation_heat
    property = crack_propagation_heat
    execute_on = timestep_end
  [../]
[]

[BCs] # unidirectional tensile boundary conditions
  [./symmx_left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmx_right]
    type = PresetBC
    variable = disp_x
    boundary = right
    value = 0
  [../]
  [./symmy_bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmy_top]
    type = PresetBC
    variable = disp_y
    boundary = top
    value = 0
  [../]
  [./symmz_back]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tfunc_z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = tfunc
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = PFFracBulkRateMaterial
    gc = 2.0
  [../]
  [./elastic]
    type = LinearIsoElasticPFDamage
    c = c
    kdamage = 1e-8
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1.0 0.5'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./crack_heat_material]
    type = ComputeCrackPropagationHeatEnergy
    c = c
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1.0'
  [../]
  [./specific_heat]
    type = GenericConstantMaterial
    prop_names = 'specific_heat'
    prop_values = '1.0'
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

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'

  dt = 1.0
  dtmin = 1.0
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
