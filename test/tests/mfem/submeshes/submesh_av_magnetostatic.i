# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3.

wire_domain = 1
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
    submesh = wire
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
  [electric_potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
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
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [tangential_a_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    boundary = ${exterior_bdr}
  []

  [top]
    type = MFEMScalarDirichletBC
    variable = electric_potential
    boundary = ${high_terminal_bdr}
    coefficient = 1.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = electric_potential
    boundary = ${low_terminal_bdr}
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
  []
  [source]
    type = MFEMMixedVectorGradientKernel
    trial_variable = electric_potential
    variable = a_field
  []  
  [diff]
    type = MFEMDiffusionKernel
    variable = electric_potential
  []
[]

[Solver]
  type = MFEMMUMPS
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Magnetostatic
    vtk_format = ASCII
  []
  [SubmeshParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MagnetostaticSubmesh
    vtk_format = ASCII
    submesh = wire
  []  
[]
