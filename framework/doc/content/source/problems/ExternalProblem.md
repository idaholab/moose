# ExternalProblem

Intermediate class that should be used as an extension point (inheritance) for externally
coupled codes (MOOSE-Wrapped Apps). The external coupling interface should override the
"solve" method along with a method for testing convergence of the last solve.

!bibtex bibliography
