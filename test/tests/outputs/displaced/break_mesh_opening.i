# The upper block moves away from the lower block across a BreakMeshByBlockGenerator cut, and
# Exodus output starts only after the cut has opened, so the distributed displaced mesh is first
# gathered to rank 0 for output while the two sides of the cut no longer touch.
[Mesh]
  parallel_type = distributed
  displacements = 'disp_x disp_y'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [upper]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0 0.5 0'
    top_right = '1 1 0'
    block_id = 1
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = upper
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [open_cut]
    type = FunctionAux
    variable = disp_y
    function = 't'
    block = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
[]

[Outputs]
  [out]
    type = Exodus
    use_displaced = true
    # Keep a single output file; the mesh is only displaced, not changed
    sequence = false
    start_step = 2
  []
[]
