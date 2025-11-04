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
    NPolar = 2
    NAzmthl = 3
    NA = 1
  []
[]

[Materials]
  [matid]
    type = MixedMatIDNeutronicsMaterial
    block = 'RGMB_CORE'
    isotopes = 'pseudo'
  []
[]
