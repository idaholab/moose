Release Log: MOOSE Version 2024-02-12

1. Date of test results: Jan 25, 2024

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/cee3f610f9cd63eb9ae514a183223c3f0ca25b8f/
   master results: https://civet.inl.gov/sha_events/idaholab/moose/b22396b32598b7f6eb83e739d65cc1ac013e7895/ 

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/tree/1300a1d0c3e32ae24e04b4fb117232cc3cc53c33

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.

   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/a96f3cc2b34ca55a09b951ed86b2f0d1247c37df/

4. Evidence that any unexpected or unintended test results have been dispositioned.

   - A single unexpected failure occured during the update of the IAPWS95 repository. This is due to
     an update from another job which ran simultaneously. This race condition has nothing to do with 
     the software testing.

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
