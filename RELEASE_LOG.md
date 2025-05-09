Release Log: MOOSE Version 2025-05-09

1. Date of test results: April 26, 2025

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/7f915d0ff61581d60455f7f415fe842cfa58ca98/
   master results: https://civet.inl.gov/sha_events/idaholab/moose/9a40aeb4d2c7912919cddac4241a3bc1ccdca7c2/

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/tree/161efa91e5419f769e05ea38aacc3ea52df8e0a3

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.
   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/fa7284a7ef58b9587173983991a96395fff21710
   
4. Evidence that any unexpected or unintended test results have been dispositioned.
   - Devel and Next: All test successful
   - Master: All tests pass, but there were a few submodule update failures. This is not a concern for a release.

5. Any actions taken in connection with any deviations from this plan.
   There are no known deviations from PLN-4005, Rev. 10.

6. A list of all CIs, as defined in Section 7.1, with the following exceptions: (1) CIs in the
   repository and (2) software libraries. The ability to list the CIs in the exceptions is inherent to
   the version control system.
   - Software quality assurance plan (SQAP): INL, PLN-4005, Rev. 10, 2025/03/04
   - Safety-software determination (SSD): INL, SSD-000709, Rev. 3
   - Quality-level determination (QLD): INL, ALL-000875, Rev. 1
   - Capabilities & Technology Management System: "Multiphysics Object Oriented Simulation Environment (MOOSE)"
     with ID APM0002312

7. Acceptability.
   The results are acceptadble for release.
