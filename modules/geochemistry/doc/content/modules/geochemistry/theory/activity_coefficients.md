# Activity coefficients

!alert note
Only the Debye-Huckel B-dot model along with the formulae below for neutral species and water are coded into the `geochemistry` module.  The virial Pitzer/HMW models have not yet been included.

Notation and definitions are described in [geochemistry_nomenclature.md].

The material below is taken largely from Chapter 8 of [!cite](bethke_2007).

Recall that the activity, $a$, of a species is defined in terms of the chemical potential, $\mu$
\begin{equation}
\mu = \mu^{0} + RT \log a \ .
\end{equation}
For minerals in mineralisation reactions, $a=1$, but for species in equilibrium reactions, the activity is a dimensionless measure of effective concentration of a constituent in a solution
\begin{equation}
a = \gamma m/m_{\mathrm{ref}} \ ,
\end{equation}
where $\gamma$ is the activity coefficient, $m$ is the molality, and $m_{\mathrm{ref}}=1\,$mol.kg$^{-1}$.  It is the purpose of this page to describe models for computing the activity coefficient $\gamma$.

## Ionic strength and stoichiometric ionic strength

Denote the "ionic strength" of the solution by $I$.  Its units are mol.kg$^{-1}$:
\begin{equation}
I = \frac{1}{2}\sum_{i}m_{i}z_{i}^{2} \ .
\end{equation}
Here, the sum runs over the free ions in the solution, so neutral complexes are not counted.

Denote the "stoichiometric ionic strength" of the solution by $I_{s}$.  Its units are mol.kg$^{-1}$:
\begin{equation}
I_{s} = \frac{1}{2}\sum_{i}m_{i}z_{i}^{2} \ .
\end{equation}
Here, complete dissociation of complexes is assumed, and then the sum runs over all ions in the hypothetical solution.

The ionic strength is computed by a `GeochemistryIonicStrength` utility, which is constructed automatically by the [GeochemistryTimeIndependentReactor](GeochemistryTimeIndependentReactor.md), [GeochemistryTimeDependentReactor](GeochemistryTimeDependentReactor.md) or [GeochemistrySpatialReactor](GeochemistrySpatialReactor.md).  These UserObjects include flags to compute the two ionic strengths using the basis components only, and/or compute the stoichiometric ionic strength using the Cl$^{-}$ molality only, both of which are common practice in geochemical modelling.  The UserObjects also allow setting the maximum ionic strength.  If $I$ and/or $I_{s}$ exceed this bound, then the maximum ionic strength is used in the formulae below, which prevents models from using the Debye-Huckel formulae for highly concentrated solutions where they are not applicable.  Since ionic strength adds considerable nonlinearity to geochemical models, the UserObjects also have the option of slowly increasing the maximum-permissible ionic strength.

## Debye-Huckel B-dot model

### Electrically-charged species

Here (see Eqn(8.5) of [!cite](bethke_2007))
\begin{equation}
\log_{10}\gamma = -\frac{Az^{2}\sqrt{I}}{1 + \mathring{a}B\sqrt{I}} + \dot{B}I \ .
\end{equation}
In this equation

- $z$ is the charge number of the species (if $z=0$ then see below).
- $I$ is the ionic strength of the solution
- $\mathring{a}$ is the ion size parameter, measured in Angstrom ($\mathring{A}$) and given for all primary basis species, redox couples and secondary species in the [chemical database](geochemistry/database/index.md)
- Coefficients $A$, $B$ and $\dot{B}$ depend on temperature, and are given in the [chemical database](geochemistry/database/index.md).  At at 25$^{\circ}$C: $A=0.5092\,$mol$^{-1/2}$.kg$^{1/2}$, $B=0.3283\,$mol$^{-1/2}$.kg$^{1/2}$.$\mathring{A}^{-1}$ and $\dot{B}=0.0410\,$mol$^{-1}$.kg.

### Neutral species: version 1

When $z=0$, the activity coefficients may be computed using (Eqn(8.6) of [!cite](bethke_2007))
\begin{equation}
\log_{10}\gamma = aI + bI^{2} + cI^{3} + dI^{4} \ .
\end{equation}
Here $a$, $b$, $c$ and $d$ are functions of temperature, and are tabulated in [chemical databases](geochemistry/database/index.md) (assuming $I$ is measured in mol.kg$^{-1}$).  Also, the database must contain a keyword, such as $\mathring{a}=-0.5$, that instructs the solver to use this expression instead of the standard Debye-Huckel B-dot model (see Section 3.1.4 of [!cite](gwb_reference)).

### Neutral species: version 2

Alternativfely, the activity coefficients for $z=0$ are computed using
\begin{equation}
\log_{10}\gamma = \dot{B}I \ .
\end{equation}
If so, the [chemical database](geochemistry/database/index.md) should contain a keyword $\mathring{a}=-1$ to instruct the solver to use this expression instead of the standard Debye-Huckel B-dot model  (see Section 3.1.4 of [!cite](gwb_reference)).

## Activity of water

The activity of water is (Eqns(8.7) and (8.8) of [!cite](bethke_2007))
\begin{equation}
\log a_{w} = -\frac{2I_{s}}{55.51} \left(
1 - \frac{A\log 10}{\tilde{a}^{3}I_{s}}\left[ \hat{b}-2\log\hat{b} - \frac{1}{\hat{b}}\right] + \frac{\tilde{b}I_{s}}{2} + \frac{2\tilde{c}I_{s}^{2}}{3} + \frac{3\tilde{d}I_{s}^{3}}{4}
\right) \ ,
\end{equation}
with
\begin{equation}
\hat{b} = 1 + \tilde{a}\sqrt{I_{s}} \ .
\end{equation}
In these expression

- $\log$ is the natural logarithm
- $I_{s}$ \[mol.kg$^{-1}$\] is the stoichiometric ionic strength
- $55.51\,$mol.kg$^{-1}$ is the number of moles in a kg of water
- $A$ \[kg$^{1/2}$.mol$^{-1/2}$\] is the standard Debye-Huckel $A$ parameter
- $\tilde{a}$, $\tilde{b}$, $\tilde{d}$ and $\tilde{d}$ are temperature-dependent coefficients.

The values of $\tilde{a}$, $\tilde{b}$, $\tilde{d}$ and $\tilde{d}$ are given in the [chemical database](geochemistry/database/index.md), assuming units of molality.  At 25$^{\circ}$C

- $\tilde{a}=1.45397\,$kg$^{1/2}$.mol$^{-1/2}$,
- $\tilde{b}=0.022357\,$kg.mol$^{-1}$,
- $\tilde{c}=0.0093804\,$kg$^{2}$.mol$^{-2}$,
- $\tilde{d}=-0.0005362\,$kg$^{3}$.mol$^{-3}$.

!bibtex bibliography
