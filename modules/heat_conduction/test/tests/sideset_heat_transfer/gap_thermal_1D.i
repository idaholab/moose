[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmax = 2
  []
  [split]
    type = SubdomainBoundingBoxGenerator
    input = mesh
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 0 0'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = split
    primary_block = 1
    paired_block = 0
    new_boundary = 'interface0'
  []
  uniform_refine = 4
[]

[Variables]
  # Defining a DFEM variable to handle gap discontinuity
  [T]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxVariables]
  # Auxvariable containing bulk temperature of gap
  [Tbulk]
    order = FIRST
    family = LAGRANGE
    initial_condition = 300 # K
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = T
    diffusivity = conductivity
  []
  [source]
    type = BodyForce
    variable = T
    value = 1.0
  []
[]

[DGKernels]
  # DG kernel to represent diffusion accross element faces
  [./dg_diff]
    type = DGDiffusion
    variable = T
    epsilon = -1
    sigma = 6
    diff = conductivity
    # Ignoring gap side set because no diffusion accross there
    exclude_boundary = 'interface0'
  [../]
[]

[InterfaceKernels]
  active = 'gap'
  # Heat transfer kernel using Tbulk as material
  [gap]
    type = SideSetHeatTransferKernel
    variable = T
    neighbor_var = T
    boundary = 'interface0'
  []
  # Heat transfer kernel using Tbulk as auxvariable
  [gap_var]
    type = SideSetHeatTransferKernel
    variable = T
    neighbor_var = T
    boundary = 'interface0'
    Tbulk_var = Tbulk
  []
[]

[Functions]
  [bc_func]
    type = ConstantFunction
    value = 300
  []
  [exact]
    type = ParsedFunction
    expression = '
            A := if(x < 1, -0.5, -0.25);
            B := if(x < 1, -0.293209850655001, 0.0545267662299068);
            C := if(x < 1, 300.206790149345, 300.19547323377);
            d := -1;
            A * (x+d) * (x+d) + B * (x+d) + C'
  []
[]

[BCs]
  [bc_left]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'left'
    variable = T
    diff = 'conductivity'
    epsilon = -1
    sigma = 6
    function = bc_func
  []
  [bc_right]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'right'
    variable = T
    diff = 'conductivity'
    epsilon = -1
    sigma = 6
    function = bc_func
  []
[]

[Materials]
  [k0]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 1.0
    block = 0
  []
  [k1]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 2.0
    block = 1
  []
  [gap_mat]
    type = SideSetHeatTransferMaterial
    boundary = 'interface0'
    conductivity = 1.5
    gap_length = 1.0
    h_primary = 1
    h_neighbor = 1
    Tbulk = 300
    emissivity_primary = 1
    emissivity_neighbor = 1
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = T
    function = exact
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
