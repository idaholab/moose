# Direct compiled audit of generic porous-LPS kernels owned by solid_mechanics.
# The test-only material derives directly from PorousViscoplasticityStressUpdate so these checks do
# not depend on BISON fission-gas pressure, topology, or inventory behavior.

!include lps_single.i

[Materials/lps]
  type := PorousViscoplasticityStressUpdateTest
  run_kernel_checks = true
  minimum_porosity = 0.05
[]

[Outputs]
  csv := false
  exodus := false
[]
