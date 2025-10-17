# Low frequency Maxwell problem solved using an AV formulation.

wire_domain = 1
vacuum_domain = 2

high_terminal_bdr = 4
low_terminal_bdr = 6
exterior_bdr = '1 2 3'

vacuum_permeability = 1.25663706e-6
vacuum_reluctivity = ${fparse 1.0/vacuum_permeability}
vacuum_conductivity = 1.0
wire_conductivity = 5.96e7

source_current_density = 1.e6

[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/wire.gen
[]

[SubMeshes]
  [wire]
    type = MFEMDomainSubMesh
    block = ${wire_domain}
  []
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
  [WireH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = wire
  []
  [WireHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = wire
  []
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
    time_derivative = da_dt
  []
  [time_integrated_electric_potential]
    type = MFEMVariable
    fespace = H1FESpace
    time_derivative = electric_potential
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
  [grad_electric_potential]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [wire_electric_potential]
    type = MFEMVariable
    fespace = WireH1FESpace
  []
  [wire_grad_electric_potential]
    type = MFEMVariable
    fespace = WireHCurlFESpace
  []  
[]

[AuxKernels]
  [update_b_field]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
  [update_grad_v]
    type = MFEMGradAux
    variable = wire_grad_electric_potential
    source = wire_electric_potential
    scale_factor = 1.0
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []
  [update_e_field]
    type = MFEMSumAux
    variable = e_field
    source_variables = 'da_dt grad_electric_potential'
    scale_factors = '-1.0 -1.0'
    execute_on = TIMESTEP_END
    execution_order_group = 4
  []
[]

[Functions]
  [exterior_a_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = '0'
  []
  [potential_diff]
    type = ParsedFunction
    expression = '10.'
  []
[]

[BCs]
  [tangential_a_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = exterior_a_field
    boundary = ${exterior_bdr}
  []
  [high_terminal]
    type = MFEMBoundaryIntegratedBC
    variable = time_integrated_electric_potential
    boundary = ${high_terminal_bdr}
    coefficient = ${source_current_density}
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = electric_potential
    boundary = ${low_terminal_bdr}
    coefficient = 0.0
  []  
[]

[FunctorMaterials]
  [Vacuum]
    type = MFEMGenericFunctorMaterial
    prop_names = 'reluctivity conductivity'
    prop_values = '${vacuum_reluctivity} ${vacuum_conductivity}'
    block = ${vacuum_domain}
  []
  [Wire]
    type = MFEMGenericFunctorMaterial
    prop_names = 'reluctivity conductivity'
    prop_values = '${vacuum_reluctivity} ${wire_conductivity}'
    block = ${wire_domain}
  []
[]

[Kernels]
  [nu.curlA,curldA'_dt]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = reluctivity
  []
  [sigma.dA_dt,dA'_dt]
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = a_field
    coefficient = conductivity
  []
  [sigma.gradV,dA'_dt]
    type = MFEMMixedVectorGradientKernel
    trial_variable = electric_potential
    variable = a_field
    coefficient = conductivity
    block = ${wire_domain}
  []
  [sigma.dA_dt,gradV']
    type = MFEMTimeDerivativeMixedVectorGradientKernel
    trial_variable = a_field
    variable = time_integrated_electric_potential
    coefficient = conductivity
    block = ${wire_domain}
    transpose = true
  []
  [sigma.gradV,gradV']
    type = MFEMMixedGradGradKernel
    trial_variable = electric_potential
    variable = time_integrated_electric_potential
    coefficient = conductivity
    block = ${wire_domain}
  []
  [V_vacuum] # Penalty on V in vacuum region
    type = MFEMMixedGradGradKernel
    trial_variable = electric_potential
    variable = time_integrated_electric_potential
    coefficient = 1e-20
    block = ${vacuum_domain}
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

[Transfers]
  [transfer_to_wire_submesh]
    type = MFEMSubMeshTransfer
    from_variable = electric_potential
    to_variable = wire_electric_potential
    execute_on = TIMESTEP_END
    execution_order_group = 1
  []
  [transfer_from_wire_submesh]
    type = MFEMSubMeshTransfer
    from_variable = wire_grad_electric_potential
    to_variable = grad_electric_potential
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []  
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/AVMagnetodynamic
  []
[]
