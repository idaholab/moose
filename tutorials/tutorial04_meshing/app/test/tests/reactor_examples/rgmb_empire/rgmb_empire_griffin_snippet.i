### This file contains a snippet from a Griffin input file. This file is not runnable in this form. ###

[TransportSystems]
  particle = neutron
  G = 6
  VacuumBoundary = 'outer_core'
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
    block = 'RGMB_CORE RGMB_CORE_TRI'
    isotopes = 'pseudo'
  []
[]
