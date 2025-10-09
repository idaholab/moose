# Low frequency Maxwell problem solved using an AV formulation.

coil_domain = 1
vacuum_domain = 2
high_terminal_bdr = 1
low_terminal_bdr = 2
exterior_bdr = '1 2 3'

[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [time_integrated_electric_potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []  
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxKernels]
  [curl]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
  [grad_potential]
    type = MFEMGradAux
    variable = e_field
    source = time_integrated_electric_potential
    scale_factor = -1.0
    execute_on = TIMESTEP_END
  []  
[]

[Functions]
  [exact_a_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = '0'
  []
  [potential_diff]
    type = ParsedFunction
    expression = '10. * t'
  []
[]

[BCs]
  [tangential_a_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = exact_a_field
    boundary = ${exterior_bdr}
  []
  [high_terminal]
    type = MFEMScalarDirichletBC
    variable = time_integrated_electric_potential
    boundary = ${high_terminal_bdr}
    coefficient = potential_diff
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = time_integrated_electric_potential
    boundary = ${low_terminal_bdr}
    coefficient = conductivity
  []  
[]

[FunctorMaterials]
  [Vacuum]
    type = MFEMGenericFunctorMaterial
    prop_names = 'reluctivity conductivity'
    prop_values = '1.0e-7 1.0'
    block = ${vacuum_domain}
  []
  [Coil]
    type = MFEMGenericFunctorMaterial
    prop_names = 'reluctivity conductivity'
    prop_values = '1.0e-7 1.0e7'
    block = ${coil_domain}
  []  
[]

[Kernels]
  [curlA,curldA'_dt]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = reluctivity
  []
  [sigmadA_dt,dA'_dt]
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = a_field
    coefficient = conductivity
  []
  [sigmagradV,dA'_dt]
    type = MFEMTimeDerivativeMixedVectorGradientKernel
    trial_variable = time_integrated_electric_potential
    variable = a_field
    coefficient = conductivity
    block = ${coil_domain}
  []

  [sigmadA_dt,gradV']
    type = MFEMTimeDerivativeMixedVectorGradientKernel
    trial_variable = a_field
    variable = time_integrated_electric_potential
    coefficient = conductivity
    block = ${coil_domain}
    transpose = true
  []  
  [gradV,gradV']
    type = MFEMTimeDerivativeDiffusionKernel
    variable = time_integrated_electric_potential
    coefficient = conductivity
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 5.0
  start_time = 0.0
  end_time = 60.0  
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/AVMagnetodynamic
  []
[]
