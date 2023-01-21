# Same problem as in moose/test/tests/kernels/simple_transient_diffusion

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./c]
  [../]
  [./mu]
  [../]
  [./jx]
  [../]
  [./jy]
  [../]
[]

[Kernels]
  [./conc]
    type = CHSplitConcentration
    variable = c
    mobility = mobility_prop
    chemical_potential_var = mu
  [../]
  [./chempot]
    type = CHSplitChemicalPotential
    variable = mu
    chemical_potential_prop = mu_prop
    c = c
  [../]
  [./flux_x]
    type = CHSplitFlux
    variable = jx
    component = 0
    mobility_name = mobility_prop
    mu = mu
    c = c
  [../]
  [./flux_y]
    type = CHSplitFlux
    variable = jy
    component = 1
    mobility_name = mobility_prop
    mu = mu
    c = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./chemical_potential]
    type = DerivativeParsedMaterial
    block = 0
    property_name = mu_prop
    coupled_variables = c
    expression = 'c'
    derivative_order = 1
  [../]
  [./var_dependence]
    type = DerivativeParsedMaterial
    block = 0
    expression = '0.1'
    coupled_variables = c
    property_name = var_dep
    derivative_order = 1
  [../]
  [./mobility_tensor]
    type = ConstantAnisotropicMobility
    block = 0
    M_name = mobility_tensor
    tensor = '1 0 0 0 1 0 0 0 1'
  [../]
  [./mobility]
    type = CompositeMobilityTensor
    block = 0
    M_name = mobility_prop
    tensors = mobility_tensor
    weights = var_dep
    args = c
  [../]
[]

[BCs]
  [./leftc]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0
  [../]
  [./rightc]
    type = DirichletBC
    variable = c
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'

  nl_max_its = 5
  dt = 0.1
  num_steps = 20
[]

[Preconditioning]
  [./smp]
     type = SMP
     full = true
  [../]
[]

[Outputs]
  exodus = true
[]
