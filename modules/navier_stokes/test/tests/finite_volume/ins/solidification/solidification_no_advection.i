rho_solid = 1.0
rho_liquid = 1.0
k_solid = 0.03
k_liquid = 0.1
cp_solid = 1.0
cp_liquid = 1.0
T_liquidus = 260
T_solidus = 240
L = 1.0

T_hot = 300.0
T_cold = 200.0
N = 10

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${N}
    ny = ${N}
  []
[]

[AuxVariables]
  [fl]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [density]
    type = MooseVariableFVReal
  []
  [th_cond]
    type = MooseVariableFVReal
  []
  [cp_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [compute_fl]
    type = NSLiquidFractionAux
    variable = fl
    temperature = T
    T_liquidus = '${T_liquidus}'
    T_solidus = '${T_solidus}'
    execute_on = 'TIMESTEP_END'
  []
  [rho_out]
    type = ADFunctorElementalAux
    functor = 'rho_mixture'
    variable = 'density'
  []
  [th_cond_out]
    type = ADFunctorElementalAux
    functor = 'k_mixture'
    variable = 'th_cond'
  []
  [cp_out]
    type = ADFunctorElementalAux
    functor = 'cp_mixture'
    variable = 'cp_var'
  []
[]

[Variables]
  [T]
    type = INSFVEnergyVariable
    initial_condition = '${T_hot}'
  []
[]

[FVKernels]
  [T_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    cp = ${cp_liquid}
    rho = ${rho_liquid}
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = 'k_mixture'
    variable = T
  []
  [energy_source]
    type = NSFVPhaseChangeSource
    variable = T
    L = ${L}
    liquid_fraction = fl
    T_liquidus = ${T_liquidus}
    T_solidus = ${T_solidus}
    rho = 'rho_mixture'
  []
[]

[FVBCs]
  [heated_wall]
    type = FVDirichletBC
    variable = T
    value = '${T_hot}'
    boundary = 'top'
  []
  [cooled_wall]
    type = FVDirichletBC
    variable = T
    value = '${T_cold}'
    boundary = 'bottom'
  []
[]

[Materials]
  [eff_cp]
    type = NSFVMixtureMaterial
    phase_2_names = '${cp_solid} ${k_solid} ${rho_solid}'
    phase_1_names = '${cp_liquid} ${k_liquid} ${rho_liquid}'
    prop_names = 'cp_mixture k_mixture rho_mixture'
    phase_1_fraction = fl
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 50.0
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-12
  nl_max_its = 50
  steady_state_detection = true
[]

[Outputs]
  exodus = true
[]
