[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
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
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [cylindrical]
    type = MFEMCoordinateTransformations
    coord_type = RZ
    inv_r_eps = 0
  []
[]

[FunctorMaterials]
  [material]
    type = MFEMGenericFunctorMaterial
    prop_names = 'diffCoef massCoef'
    prop_values = 'cylindrical_r cylindrical_inv_r'
  []
[]

[BCs]
  [Dirichlet]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 10
  []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = u
    coefficient = diffCoef
  []

  [mass]
    type = MFEMMassKernel
    variable = u
    coefficient = massCoef
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/CylindricalCoefficients
    scalar_coefficients = 'cylindrical_inv_r cylindrical_two_pi_r'
    vtk_format = ASCII
  []
[]
