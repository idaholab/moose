[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 2
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = 'x0:=5.0;thk:=0.5;m:=2;r:=abs(x-x0);v:=exp(-(r/thk)^m);0.1+0.1*v'
    [../]
  [../]
  [./mu]
  [../]
[]

[AuxVariables]
  [./gb]
    family = LAGRANGE
    order  = FIRST
  [../]
  [./mobility_xx]
    family = MONOMIAL
    order  = CONSTANT
  [../]
  [./mobility_yy]
    family = MONOMIAL
    order  = CONSTANT
  [../]
  [./diffusivity_xx]
    family = MONOMIAL
    order  = CONSTANT
  [../]
  [./diffusivity_yy]
    family = MONOMIAL
    order  = CONSTANT
  [../]
  [./aniso_tensor_xx]
    family = MONOMIAL
    order  = CONSTANT
  [../]
  [./aniso_tensor_yy]
    family = MONOMIAL
    order  = CONSTANT
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
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[AuxKernels]
  [./gb]
    type = FunctionAux
    variable = gb
    function = 'x0:=5.0;thk:=0.5;m:=2;r:=abs(x-x0);v:=exp(-(r/thk)^m);v'
  [../]
  [./mobility_xx]
    type = MaterialRealTensorValueAux
    variable = mobility_xx
    property = mobility_prop
    row = 0
    column = 0
  [../]
  [./mobility_yy]
    type = MaterialRealTensorValueAux
    variable = mobility_yy
    property = mobility_prop
    row = 1
    column = 1
  [../]
  [./diffusivity_xx]
    type = MaterialRealTensorValueAux
    variable = diffusivity_xx
    property = diffusivity
    row = 0
    column = 0
  [../]
  [./diffusivity_yy]
    type = MaterialRealTensorValueAux
    variable = diffusivity_yy
    property = diffusivity
    row = 1
    column = 1
  [../]
  [./aniso_tensor_xx]
    type = MaterialRealTensorValueAux
    variable = aniso_tensor_xx
    property = aniso_tensor
    row = 0
    column = 0
  [../]
  [./aniso_tensor_yy]
    type = MaterialRealTensorValueAux
    variable = aniso_tensor_yy
    property = aniso_tensor
    row = 1
    column = 1
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
    expression = 'c*(1.0-c)'
    coupled_variables = c
    property_name = var_dep
    derivative_order = 1
  [../]
  [./mobility]
    type = CompositeMobilityTensor
    block = 0
    M_name = mobility_prop
    tensors = diffusivity
    weights = var_dep
    args = c
  [../]
  [./phase_normal]
    type = PhaseNormalTensor
    phase = gb
    normal_tensor_name = gb_normal
  [../]
  [./aniso_tensor]
    type = GBDependentAnisotropicTensor
    gb = gb
    bulk_parameter = 0.1
    gb_parameter = 1
    gb_normal_tensor_name = gb_normal
    gb_tensor_prop_name = aniso_tensor
  [../]
  [./diffusivity]
    type = GBDependentDiffusivity
    gb = gb
    bulk_parameter = 0.1
    gb_parameter = 1
    gb_normal_tensor_name = gb_normal
    gb_tensor_prop_name = diffusivity
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 20
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'

  l_tol = 1e-3
  l_max_its = 20
  nl_max_its = 5
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
