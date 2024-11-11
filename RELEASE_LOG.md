Release Log: MOOSE Version 2024-11-11

1. Date of test results: October 29, 2024

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/9c9cc0f741b9506906fd4ddb7c05c4293128bff4/ 
   master results: https://civet.inl.gov/sha_events/idaholab/moose/5099b78fb4e949f7db21cbcece772e651d78be21/

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/commit/52ab6d60b52b7fc9e7fd37b1a9ff93fb83a575e5

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.
   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/c34e99a75f541a01b5f02cf4e88210f8b7da53c8

4. Evidence that any unexpected or unintended test results have been dispositioned.
   - Install dire_wolf (Linux): Direwolf failed to install due to a documentation problem. This problem was evaluated and
     is unrelated to MOOSE. 
   - Copy and run dire_wolf tests: Failed due to the install target failing - unrelated to MOOSE.
   - HPC release moose-dev: Snapshot container was not released to the HPC - this system is still being developed, not related to the release.
   - Docker moose: Docker snapshot failed to build - this is not related to MOOSE.  

5. Any actions taken in connection with any deviations from this plan.
   There are no known deviations from PLN-4005, Rev. 9.

6. A list of all CIs, as defined in Section 7.1, with the following exceptions: (1) CIs in the
   repository and (2) software libraries. The ability to list the CIs in the exceptions is inherent to
   the version control system.
   - Software quality assurance plan (SQAP): INL, PLN-4005, Rev. 9, 2022/01/31
   - Safety-software determination (SSD): INL, SSD-000709, Rev. 3
   - Quality-level determination (QLD): INL, ALL-000875, Rev. 1
   - Capabilities & Technology Management System: "Multiphysics Object Oriented Simulation Environment (MOOSE)
     with ID APM0002312

7. Acceptability.
   The results are acceptadble for release.
