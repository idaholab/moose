[Mesh]
    type = MFEMMesh
    file = ../mesh/star.mesh
[]

[Problem]
    type = MFEMProblem
[]

[CoordinateSystem]
    [cylindrical]
        type = MFEMCylindrical
        inv_r_eps = 1e-100
    []
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
    [u_exact]
        type = ParsedFunction
        expression = 10
    []
[]

[FunctorMaterials]
    [material]
        type = MFEMGenericFunctorMaterial
        prop_names = 'diffCoef massCoef'
        prop_values = 'cylindrical_r cylindrical_inv_r'
        block =  1
    []
[]

[BCs]
    [Dirichlet]
        type = MFEMScalarDirichletBC
        variable = u
        boundary = '1'
        coefficient =u_exact
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
    file_base = OutputData/CylindricalCoefficients
    scalar_coefficients = 'cylindrical_inv_r cylindrical_two_pi_r'
    vtk_format = ASCII
  []
[]
