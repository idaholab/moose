Release Log: MOOSE Version 2024-03-08

1. Date of test results: March 8, 2024

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/d9f4aef184f108702810ace1fb8b957e6afd4815/
   master results: https: https://civet.inl.gov/sha_events/idaholab/moose/e7a20f90eb09e04b43793acf2b157d5f1657c7dd/

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/tree/6fc782799cf3cdf1bcf5c4bf22bfea27a8940785

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.

   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/f4f49d21ae0dc0293e7496bd140332a9fba856ff

4. Evidence that any unexpected or unintended test results have been dispositioned.

   - Conda build (Linux): A YAML file was missing causing the error due to a recipe change. This did not cause
     the conda environment to fail as is evident by the downstream dependencies that ran using this environment.
   - Controlled app Tests PETSc alt: Two minor BlueCRAB tests failed due to an ongoing MOOSE integration. 
     This failure was expected and is not a concern for the functionality of this release.
   - Controlled app Tests: The same BlueCRAB tests failed in this target as the previous bullet. Same issue.
     Additionally Mockingbird failed to build. It is not a critical application and the build error is understood.
     It is related to a change to MOOSE which has not been refactored into Mockingbird. (Not an SQA controlled application).

   - Update internal apps: Mockingbird failed to test and so was not updated. This is due to the same issue.

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
