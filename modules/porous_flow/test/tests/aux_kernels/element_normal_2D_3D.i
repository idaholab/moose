# The PorousFlowElementNormal is used to calculate normal directions
[Mesh]
  [base]
    type = AnnularMeshGenerator
    dmax = 90
    nr = 1
    nt = 1
    rmin = 0.1
    rmax = 1
  []
  [make3D]
    type = MeshExtruderGenerator
    input = base
    bottom_sideset = bottom
    extrusion_vector = '0 0 1'
    top_sideset = top
  []
  [rmax_block]
    type = LowerDBlockFromSidesetGenerator
    input = make3D
    sidesets = rmax
    new_block_name = rmax
  []
  [top_block]
    type = LowerDBlockFromSidesetGenerator
    input = rmax_block
    sidesets = top
    new_block_name = top
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
    3D_default = '-3 4 5'
  []
  [ny]
    type = PorousFlowElementNormal
    variable = ny
    component = y
    3D_default = '-3 4 5'
  []
  [nz]
    type = PorousFlowElementNormal
    variable = nz
    component = z
    3D_default = '-3 4 5'
  []
[]

[Postprocessors]
  [n3Dx]
    type = ElementAverageValue
    variable = nx
    block = 0
  []
  [n3Dy]
    type = ElementAverageValue
    variable = ny
    block = 0
  []
  [n3Dz]
    type = ElementAverageValue
    variable = nz
    block = 0
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
  [ntopx]
    type = ElementAverageValue
    variable = nx
    block = top
  []
  [ntopy]
    type = ElementAverageValue
    variable = ny
    block = top
  []
  [ntopz]
    type = ElementAverageValue
    variable = nz
    block = top
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
  exodus = true
[]
