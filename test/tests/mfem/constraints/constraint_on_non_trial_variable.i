# Error case: a constraint is applied to a variable that no kernel acts on, so the
# variable is not a trial variable of the equation system and nothing would ever
# apply the constraint.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
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
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [orphan]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]


[BCs]
  [bc]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = outer
    coefficient = 1.0
  []
[]

[Constraints]
  [orphan_constraint]
    type = MFEMScalarEssentialConstraint
    variable = orphan
    block = wire
    coefficient = 1.0
  []
  [circle_interior]
    type = MFEMScalarEssentialConstraint
    block = wire
    coefficient = 2.0
    variable = temperature
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = temperature
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-16
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMVariableLineValueSampler
    variable = 'temperature'
    start_point = '-3.0 0.0 0.0'
    end_point = '3.0 0.0 0.0'
    num_points = 21
  []
[]

[Outputs]
  csv = true
[]
