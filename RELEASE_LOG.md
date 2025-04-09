Release Log: MOOSE Version 2025-04-07

1. Date of test results: March 27, 2025

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/48c61aaa5b25379d768e22dede9ee8d18641fdd2/
   master results: https://civet.inl.gov/sha_events/idaholab/moose/7bf988912c20bc59f4ce868339984b99496eea00/

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/tree/36c03fa502f0d92887071b2c848808f10472d8f3

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.
   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/ca5752da7791dfca64d86f8226bf31e3235e5b7e
   
4. Evidence that any unexpected or unintended test results have been dispositioned.
   - BlueCRAB failures in multiple targets: BlueCRAB was being refactored at the time due to the movement of objects between
     applications. This build was occuring during the movement of these files so BlueCRAB was non-functional. BlueCRAB does
     not build with this version of MOOSE, but is released separately.
   - Ba Hun, has a single failing test that is not maintained by us (Australian application). It has since been rectified.
   - Ferret also failed to build failure in this version of MOOSE. It has since been rectified. 

5. Any actions taken in connection with any deviations from this plan.
   There are no known deviations from PLN-4005, Rev. 10.

6. A list of all CIs, as defined in Section 7.1, with the following exceptions: (1) CIs in the
   repository and (2) software libraries. The ability to list the CIs in the exceptions is inherent to
   the version control system.
   - Software quality assurance plan (SQAP): INL, PLN-4005, Rev. 10, 2025/03/04
   - Safety-software determination (SSD): INL, SSD-000709, Rev. 3
   - Quality-level determination (QLD): INL, ALL-000875, Rev. 1
   - Capabilities & Technology Management System: "Multiphysics Object Oriented Simulation Environment (MOOSE)
     with ID APM0002312

7. Acceptability.
   The results are acceptadble for release.
