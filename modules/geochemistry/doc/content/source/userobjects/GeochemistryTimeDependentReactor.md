# GeochemistryTimeDependentReactor

This UserObject is usually added via a [TimeDependentReactionSolver](TimeDependentReactionSolver/index.md) Action: please see that page for many examples.  This UserObject's purpose is to solve a time-dependent geochemical system.

Advanced users may wish to add `GeochemistryTimeDependentReactor` objects manually to their input files.  Here is an example of that:

!listing test/tests/time_dependent_reactions/simple_no_action.i

!syntax parameters /UserObjects/GeochemistryTimeDependentReactor

!syntax inputs /UserObjects/GeochemistryTimeDependentReactor

!syntax children /UserObjects/GeochemistryTimeDependentReactor

