# Testing Jacobian resulting from PorousFlowPorosityLinear in a THM situation
[GlobalParams]
  PorousFlowDictator = dictator
  strain_at_nearest_qp = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[Variables]
  [pp]
    initial_condition = 1
  []
  [T]
    initial_condition = 2
  []
  [disp]
  []
[]

[ICs]
  [disp]
    type = FunctionIC
    variable = disp
    function = '3 * x'
  []
[]

[BCs]
  [disp]
    type = FunctionDirichletBC
    boundary = 'left right top bottom front back'
    variable = disp
    function = '3 * x'
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydroMechanical
  fp = simple_fluid
  porepressure = pp
  temperature = T
  displacements = 'disp disp disp'
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityLinear
    porosity_ref = 0.5
    P_ref = 0.5
    P_coeff = 1.0
    T_ref = -3.0
    T_coeff = 1.0
    epv_ref = 2.5
    epv_coeff = 1.0
  []
  [perm]
    type = PorousFlowPermeabilityConst
    permeability = '0 0 0  0 0 0  0 0 0'
  []
  [matrix_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 0.0
    specific_heat_capacity = 0.0
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0 0 0  0 0 0  0 0 0'
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 0.0
  []
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1E-99
    poissons_ratio = 0
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp disp disp'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
#  petsc_options = '-snes_test_jacobian -snes_force_iteration'
#  petsc_options_iname = '-snes_type --ksp_type -pc_type -snes_convergence_test'
#  petsc_options_value = ' ksponly     preonly   none     skip'
[]
