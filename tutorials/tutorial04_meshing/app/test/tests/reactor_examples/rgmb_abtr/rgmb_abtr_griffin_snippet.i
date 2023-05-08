### This file contains a snippet from a Griffin input file. This file is not runnable in this form. ###

[TransportSystems]
  particle = neutron
  G = 33
  VacuumBoundary = 'outer_core top bottom'
  equation_type = eigenvalue
  [sn]
    scheme = DFEM-SN
    family = L2_LAGRANGE
    order = FIRST
    AQtype = Gauss-Chebyshev
    NPolar = 3
    NAzmthl = 4
    NA = 2
    sweep_type = asynchronous_parallel_sweeper
    using_array_variable = true
    collapse_scattering  = true
  []
[]

[Materials]
  [matid]
    type = MixedMatIDNeutronicsMaterial
    block = 'RGMB_CORE'
    isotopes = 'pseudo'
  []
[]
