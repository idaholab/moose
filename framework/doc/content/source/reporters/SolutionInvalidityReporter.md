# SolutionInvalidityReporter

## Description

The [/SolutionInvalidity.md] holds solution invalid warning information for MOOSE. This Reporter stores the summary of solution invalid warning occurrences from the [/SolutionInvalidity.md] in the Reporter value 'solution_invalidity'.

1. *object_type*: shows the moose object name registered for the solution invalid detection and an optional description
2. *converged_counts*: the number of solution invalid warnings for the latest iteration, at convergence
3. *timestep_counts*: shows the number of solution invalid warnings for the latest time step
4. *total_counts*: shows the total number of solution invalid warnings for the entire simulation
5. *Message*: shows the description of the solution invalidity

!syntax parameters /Reporters/SolutionInvalidityReporter

!syntax inputs /Reporters/SolutionInvalidityReporter

!syntax children /Reporters/SolutionInvalidityReporter
