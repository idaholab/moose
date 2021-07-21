# The PorousFlowElementNormal is used to calculate normal directions
[Mesh]
  [base]
    type = AnnularMeshGenerator
    dmax = 90
    nr = 3
    nt = 1
    rmin = 0
    rmax = 1
  []
  [rotate]
    type = TransformGenerator
    input = base
    transform = ROTATE
    vector_value = '0 45 0'
  []
  [rmax_block]
    type = LowerDBlockFromSidesetGenerator
    input = rotate
    sidesets = rmax
    new_block_name = rmax
  []
  [dmax_block]
    type = LowerDBlockFromSidesetGenerator
    input = rmax_block
    sidesets = dmax
    new_block_name = dmax
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[AuxVariables]
  [nx]
    family = MONOMIAL
    order = CONSTANT
  []
  [ny]
    family = MONOMIAL
    order = CONSTANT
  []
  [nz]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [nx]
    type = PorousFlowElementNormal
    variable = nx
    component = x
    1D_perp = '0 1 0'
  []
  [ny]
    type = PorousFlowElementNormal
    variable = ny
    component = y
    1D_perp = '0 1 0'
  []
  [nz]
    type = PorousFlowElementNormal
    variable = nz
    component = z
    1D_perp = '0 1 0'
  []
[]

[Postprocessors]
  [n2Dx]
    type = ElementAverageValue
    variable = nx
    block = '0 1'
  []
  [n2Dy]
    type = ElementAverageValue
    variable = ny
    block = '0 1'
  []
  [n2Dz]
    type = ElementAverageValue
    variable = nz
    block = '0 1'
  []
  [nrmaxx]
    type = ElementAverageValue
    variable = nx
    block = rmax
  []
  [nrmaxy]
    type = ElementAverageValue
    variable = ny
    block = rmax
  []
  [nrmaxz]
    type = ElementAverageValue
    variable = nz
    block = rmax
  []
  [ndmaxx]
    type = ElementAverageValue
    variable = nx
    block = dmax
  []
  [ndmaxy]
    type = ElementAverageValue
    variable = ny
    block = dmax
  []
  [ndmaxz]
    type = ElementAverageValue
    variable = nz
    block = dmax
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
[]
