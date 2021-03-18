# PorousFlowMassFraction

!syntax description /Materials/PorousFlowMassFraction

The `mass_fraction_vars` is a list of mass fractions, $\chi_{\beta}^{\kappa}$, of fluid component $\kappa$ in phase $\beta$:
\begin{equation}
\chi_{0}^{0},\ 
\chi_{0}^{1},\ 
\chi_{0}^{2},\ \ldots
\chi_{0}^{N-2},\ 
\chi_{1}^{0},\ 
\chi_{1}^{1},\ 
\chi_{1}^{2},\ \ldots
\chi_{1}^{N-2},\
\ldots
\chi_{P-1}^{0},\ 
\chi_{P-1}^{1},\ 
\chi_{P-1}^{2},\ \ldots
\chi_{P-1}^{N-2}
\end{equation}
There are $P$ phases and $N$ fluid components.  Notice that $\chi_{\beta}^{N-1}$ need not be specified for any phase, since
\begin{equation}
\chi_{\beta}^{N-1} = 1 - \sum_{\kappa = 0}^{N-2}\chi_{\beta}^{\kappa} \ .
\end{equation}
Examples are:

- with a single component fluid in a single-phase system, `mass_fraction_vars` should be left empty since the mass-fraction is always 1.0.
- with 3 fluid components in a single-phase system, `mass_fraction_vars = f0 f1`, where `f0` is the mass fraction of the zeroth component, and `f1` is the mass fraction of the first component (these will be AuxVariables or Variables, depending on your model).  The second component does not need to be specified since `f2 = 1 - f0 - f1`.
- with 2 fluid components in a 2-phase system, `mass_fraction_vars = f00 f10` where `f00` is the mass fraction of the zeroth component in the zeroth phase, and `f10` is the mass fraction of the zeroth component in the first phase.


!syntax parameters /Materials/PorousFlowMassFraction

!syntax inputs /Materials/PorousFlowMassFraction

!syntax children /Materials/PorousFlowMassFraction
