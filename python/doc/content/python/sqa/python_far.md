!template load file=sqa/far.md.template project=MOOSE Tools

!template! item key=introduction
{{project}}, as a collection of utilities, is designed to operate as a library of functionality. While
each utility is tailored to perform a specific set of functions, the code can be arbitrarily expanded
and built upon to create extended capability. This flexibility exists by design within the utilities,
where they are often created with a central core set of code framework surrounded by extensions built
to house specific features (the best example of this is the [MooseDocs/index.md]). With respect to
performing failure analysis, the flexibility of the code base can be detrimental since there lacks a
well-defined problem to assess. To minimize the possibility of failure for a simulation, various
automated methods exist for developers. This document discusses these features and includes a list
of requirements associated with software failure analysis.
!template-end!

!! TODO "failure-analysis" section

!template item key=failure-analysis-requirements
!sqa requirements link=True collections=FAILURE_ANALYSIS category=python
