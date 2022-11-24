# PorousFlowOutflowBC: testing Jacobian for single-phase, single-component, no heat
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '1 2 3'
[]

[Variables]
  [pp]
  []
[]

[PorousFlowFullySaturated]
  add_darcy_aux = false
  fp = simple_fluid
  porepressure = pp
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1.2
    viscosity = 0.4
  []
[]

[BCs]
  [outflow0]
    type = PorousFlowOutflowBC
    boundary = 'front back top bottom front back'
    variable = pp
    multiplier = 1E8 # so this BC gets weighted much more heavily than Kernels
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.4
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.1 0.2 0.3 1.8 0.9 1.7 0.4 0.3 1.1'
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
  dt = 1E-7
  num_steps = 1
#  petsc_options = '-snes_test_jacobian -snes_force_iteration'
#  petsc_options_iname = '-snes_type --ksp_type -pc_type -snes_convergence_test'
#  petsc_options_value = ' ksponly     preonly   none     skip'
[]
