[Mesh]
  type = MFEMMesh
  file = ../mesh/beam-tet.mesh
  displacement = displacement
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [h1_vector]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    range_dim = 3
    ordering = vdim
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = h1_vector
  []
[]

[Functions]
  [initial_displacement]
    type = ParsedVectorFunction
    # Ten percent axial strain keeps the deformation modest while making the L2 norm sensitive to
    # both the displacement field and the displaced mesh geometry.
    expression_x = '0.1*x'
    expression_y = 0
    expression_z = 0
  []
[]

[ICs]
  [displacement]
    type = MFEMVectorIC
    variable = displacement
    vector_coefficient = initial_displacement
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  # Two unit time steps exercise persistent displacement state during ordinary and recovery runs.
  dt = 1
  end_time = 2
[]

[Postprocessors]
  [displacement_norm]
    type = MFEMVectorL2Error
    variable = displacement
    function = '0 0 0'
  []
[]

[Outputs]
  csv = true
  file_base = displacement_state
[]
