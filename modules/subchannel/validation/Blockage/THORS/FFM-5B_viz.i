################################################################################
## THORS bundle 5B partial edge blockage benchmark                            ##
## SCM simulation                                                             ##
## POC : Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov             ##
################################################################################
# Details on the experimental facility modeled can be found at:
# Han, J. T. "Blockages in LMFBR fuel assemblies: A review of experimental and theoretical studies." (1977).
# This input file creates a 3D subchannel mesh that gets the subchannel information projected on,
# for visualization purposes

[Mesh]
  [subchannel]
    type = SCMDetailedTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 3.41e-2
    heated_length = 0.4572
    unheated_length_entry = 0.4064
    unheated_length_exit = 0.1524
    pin_diameter = 5.84e-3
    pitch = 7.26e-3
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime]
    block = subchannel
  []
  [displacement]
    block = subchannel
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]
