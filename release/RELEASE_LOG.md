Release Log: MOOSE Version 2023-06-13

1. Date of test results: June 13, 2023

   The following two links contain the test results for the release, these results are also included
   in the release folder of the repository.

   next:   https://civet.inl.gov/event/129275/
   Devel:  https://civet.inl.gov/event/129314/
   master: https://civet.inl.gov/event/129332/

2. Person evaluating the test results.

   Cody J. Permann

3. The support software versions and hardware configurations used for test results.

   - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
   - CIVET: https://github.com/idaholab/civet/commit/d5a85f9accd290d184480d5d651b52c0ffca1043

   The hardware configurations are contained in the included (and linked) tests results. These results
   include output from each named recipe. Each recipe reports the hardware configuration used. The
   recipe versions utilized for this release can be found in the following version of the recipes
   repository.

   - civet_recipes: https://github.com/idaholab/civet_recipes/tree/7aa2e6f05e8a3d1dbc08d62a7eb4f8b194e8ba13

4. Evidence that any unexpected or unintended test results have been dispositioned.

   - Griffin Documentation failure was expected and is not related to any framework change.
   - Submodule update error in BlueCRAB was due to a repository error and not related to any framework change.
   - Testing failure in Falcon was due to a repository configuration error and not related to any framework change.

5. Any actions taken in connection with any deviations from this plan.

   There are no known deviations from PLN-4005, Rev. 9.

6. A list of all CIs, as defined in Section 7.1, with the following exceptions: (1) CIs in the
   repository and (2) software libraries. The ability to list the CIs in the exceptions is inherent to
   the version control system.

   - Software quality assurance plan (SQAP): INL, PLN-4005, Rev. 9, 2022/01/31
   - Safety-software determination (SSD): INL, SSD-000709, Rev. 3
   - Quality-level determination (QLD): INL, ALL-000875, Rev. 1
   - Enterprise Architecture Entry: INL, "Multiphysics Object Oriented Simulation Environment (MOOSE)"
                                    with UUID A6659DD8-CF0E-4D26-AB51-F127B9FEDC66

7. Acceptability.

   The results are acceptable for release.