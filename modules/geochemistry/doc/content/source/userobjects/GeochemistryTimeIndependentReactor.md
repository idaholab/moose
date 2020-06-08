# GeochemistryTimeIndependentReactor

This UserObject is usually added via a [TimeIndependentReactionSolver](TimeIndependentReactionSolver/index.md) Action: please see that page for many examples.  This UserObject's purpose is to solve a time-independent geochemical system.

Advanced users may wish to add `GeochemistryTimeIndependentReactor` objects manually to their input files.  Here is an example of that:

!listing test/tests/equilibrium_models/HCl_no_action.i

!syntax parameters /UserObjects/GeochemistryTimeIndependentReactor

!syntax inputs /UserObjects/GeochemistryTimeIndependentReactor

!syntax children /UserObjects/GeochemistryTimeIndependentReactor

