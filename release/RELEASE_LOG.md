Release Log: MOOSE Version 2022-06-10

 1. Date of test results: May 26, 2022

    The following two links contain the test results for the release, these results are also included
    in the release folder of the repository.

    - https://civet.inl.gov/sha_events/idaholab/moose/784bb0b366607d04400396fc2e273bd91f7b1c65/
    - https://civet.inl.gov/sha_events/idaholab/moose/d37aaebc8befbdef71295a6f2ba65cc5a5b06be5/

 2. Person evaluating the test results.

    Cody J. Permann

 3. The support software versions and hardware configurations used for test results.

    - GitHub: https://github.com/idaholab/moose (This is a public website/service, there is no version number to report)
    - CIVET: https://github.com/idaholab/civet/commit/64591a1d1d6134bb7fc8cb944ea4a9beef9b7d34

    The hardware configurations are contained in the included (and linked) tests results. These results
    include output from each named recipe. Each recipe reports the hardware configuration used. The
    recipe versions utilized for this release can be found in the following version of the recipes
    repository.

    - civet_recipes: https://github.com/idaholab/civet_recipes/commit/4433daaa8a16d50135de38535f6b5046c0d90f9c

 4. Evidence that any unexpected or unintended test results have been dispositioned.

    Failures in the Apple Silicon tests are expceted. This platform is new and we are still working to fully support it.

 5. Any actions taken in connection with any deviations from this plan.

    There are no known deviations from PLN-4005, Rev. 8.

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