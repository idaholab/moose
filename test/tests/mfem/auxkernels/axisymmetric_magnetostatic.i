[Mesh]
#   [hinomaru]  
  type = MFEMFileMesh
  file = ../mesh/hinomaru_offset.e
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
  [RTFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
    basis = GaussLegendre
  []
[]

[Variables]
  [Atheta]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [zero]
    type = MFEMParsedFunction
    expression = 0
  []

  [cylindrical]
    type = MFEMCoordinateTransformations
    coord_type = RZ
    inv_r_eps = 0
  []

  # Weighted source coefficient r * J_theta used on the RHS
  [Jtheta_r_wire]
    type = ParsedFunction
    expression = 8*sqrt(x*x+y*y)
  []
[]

[FunctorMaterials]
  # Cylindrical coefficients used in the axisymmetric weak form
  [cyl_coeffs]
    type = MFEMGenericFunctorMaterial
    prop_names = 'diffCoef massCoef'
    prop_values = 'cylindrical_r cylindrical_inv_r'
  []

  # Azimuthal current density in the wire
  [J_wire]
    type = MFEMGenericFunctorMaterial
    prop_names = 'Jtheta sourceCoef'
    prop_values = '8.0 Jtheta_r_wire'
    block = wire
  [] 
[]

[AuxVariables]
  [B]
    type = MFEMVariable
    fespace = RTFESpace
  []
  [J]
    type = MFEMVariable
    fespace = L2FESpace
    []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = Atheta
    coefficient = diffCoef
  []
  [mass]
    type = MFEMMassKernel
    variable = Atheta
    coefficient = massCoef
  []

  [source_wire]
    type = MFEMDomainLFKernel
    variable = Atheta
    coefficient = sourceCoef
  []
[]

[AuxKernels]
  [B_from_Atheta]
    type = MFEMAxisymmetricCurlAthetaAux
    variable = B
    source = Atheta
    coordinate_function = cylindrical
  []
  [J]
    type = MFEMScalarProjectionAux
    variable = J
    coefficient = Jtheta
    []
[]

[BCs]
  [essential]
    type = MFEMScalarDirichletBC
    variable = Atheta
    boundary = outer
    coefficient = 1
  []
[]

[Solvers]
  [PCG]
    type = MFEMHyprePCG
    l_tol = 1e-8
  []
[]

[VectorPostprocessors]
  [LineSampler]
    type = MFEMVariableLineValueSampler
    variable = 'B'
    start_point = '2.9 -2 0'
    end_point = '2.9 2 0'
    num_points = 10
  []
[]

[Executioner]
  type = MFEMSteady
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/AxisymmetricMagnetostatic
  []
[]
