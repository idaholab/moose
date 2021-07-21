# Demonstrates the correct usage of strain_at_nearest_qp when using a nodal PorousFlowPorosity
# For the PorousFlowPorosity Material to require the strain_at_nearest_qp=true flag, it must:
#  - be a nodal Material
#  - be coupled to solid mechanics (mechanical=true)
#  - be part of a simulation with DiracKernels
# The reason for this requirement is that the volumetric strain is a standard Material (at_nodes=false)
# so that it is evaluated at the single Dirac quadpoint, and has size = 1 (assuming just one Dirac point).
# However, the PorousFlowPorosity Material will have size = 2  (number of nodes in the element containing the Dirac point).
# So when the PorousFlowPorosity Material is evaluated, it will use _vol_strain at 2 points.
# If strain_at_nearest_qp=false, then _vol_strain will be evaluated at two quadpoints, but it only has size=1, leading to a segfault
# If strain_at_nearest_qp=true, then _vol_strain will be evaluated correctly just at the single quadpoint
#
# This input file solves no useful physics: it is just illustrating the above point
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  strain_at_nearest_qp = true
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'disp_x'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [dummy_sum]
    type = PorousFlowSumQuantity
  []
[]

[Variables]
  [disp_x]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = disp_x
  []
[]

[DiracKernels]
  [line_sink]
    type = PorousFlowPolyLineSink
    function_of = temperature
    SumQuantityUO = dummy_sum
    point_file = strain_at_nearest_qp.bh
    p_or_t_vals = '0'
    fluxes = '0'
    variable = disp_x
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature  # needed because of the PorousFlowPolyLineSink
  []
  [total_strain]
    type = ComputeSmallStrain
    displacements = disp_x
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
    displacements = disp_x
  []
  [porosity_at_nodes]
    type = PorousFlowPorosity
    mechanical = true # to ensure coupling with volumetric strain
    at_nodes = true  # to ensure evaluation at nodes
    porosity_zero = 0
  []
[]


[Preconditioning]
  [usual]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  dt = 1
  solve_type = NEWTON
[]
