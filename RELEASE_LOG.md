Release Log: MOOSE Version 2026-09-25

1. Date of test results: September 25, 2026

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next/devel results: https://civet.inl.gov/sha_events/idaholab/moose/8095e1122397df3f4e736969849f9139204f031f/
   master results: https://civet.inl.gov/sha_events/idaholab/moose/e6a7269a5c8e884ea82a7b64ac687043d219022c/

2. Person evaluating the test results.

   Cody J. Permann/Guillaume Giudicelli

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/tree/7617bbfb568a833a1359c38d4dc9ba2f3049128b

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.
   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/a5ecd05d6ab858da8abf321bf712292616c131cd

4. Evidence that any unexpected or unintended test results have been dispositioned.
   - Devel and Next: Testing in Ferret, an external application, failed.
                     BlueCrab failed to build due to changes in the partitioner APIs, which were propagated in Griffin
                     but the Griffin submodule in BlueCrab has not yet been updated.
                     DireWolf failed to build for the same reason.
                     Okami failed to build due to lagging its BISON submodule and changes in the restartableData APIs

   - Master: A work-in-progress recipe for ARM testing failed.
             The HPC release, which is undergoing a rework, failed.
             The BlueCrab submodule update failed due to a missing documentation file.
             The fluid properties submodule updates failed due to a linking error.
             This is not a concern for a release.

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
   The results are acceptable for release.
