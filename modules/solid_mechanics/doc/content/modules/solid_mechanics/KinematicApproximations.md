# Kinematic Approximations

## Purpose

The strain calculator
[`ComputeLagrangianStrain`](/ComputeLagrangianStrain.md) converts the inverse
incremental deformation gradient
\begin{equation}
   f^{-1}_{ij} = F^{(n)}_{iK} \, F^{-1}_{Kj}
\end{equation}
into the spatial-velocity-gradient increment $\Delta l_{ij}$ from which it
derives the incremental strain $\Delta d_{ij} = \tfrac{1}{2}(\Delta l_{ij} + \Delta l_{ji})$
and the incremental vorticity $\Delta w_{ij} = \tfrac{1}{2}(\Delta l_{ij} - \Delta l_{ji})$.
Several closed-form approximations to this conversion are available.
The `kinematic_approximation` parameter on the strain calculator selects which
approximation is used.  Small kinematics (`large_kinematics = false`) is always
linear; this parameter only affects large kinematics.

The choice of approximation has two consumers:

- The strain increment $\Delta d$ propagated through the strain measures (and,
  after eigenstrain subtraction, fed to the constitutive model).
- The vorticity increment $\Delta w$ consumed by the
  [Jaumann, Green-Naghdi, and Rashid objective rates](/ComputeLagrangianObjectiveStress.md).
  All four objective rate options pick up whichever $\Delta w$ the strain
  calculator produces, so any pairing of approximation and rate is consistent.

## The Four Options

Let $X_{ij} = \delta_{ij} - f^{-1}_{ij}$ throughout.

### `linear` (default)

\begin{equation}
   \Delta l = X = I - f^{-1}, \quad
   \Delta d = \mathrm{sym}(X), \quad
   \Delta w = \mathrm{skew}(X).
\end{equation}

This is the first-order Taylor expansion of $-\log f^{-1}$ in $X$, and is the
default.

### `quadratic`

\begin{equation}
   \Delta l = X + \tfrac{1}{2} X^2.
\end{equation}

One additional term in the Taylor expansion of $-\log f^{-1}$.  Provided for
completeness; there is no specific compatibility target.

### `rashid_approximate`

Rashid's symmetric/skew split [!cite](rashid1993incremental).  With
\begin{equation}
   A_{ij} = X_{ik} X_{jk} - X_{ij} - X_{ji}
\end{equation}
the strain increment is
\begin{equation}
   \Delta d = -\tfrac{1}{2} A + \tfrac{1}{4} A^2.
\end{equation}
The vorticity is built from the axial vector $\alpha_k = \epsilon_{kab} f^{-1}_{ab}$:
\begin{equation}
   \Delta w_{ij} = -\frac{\theta}{2 \sqrt{Q}}\, \epsilon_{ijk}\, \alpha_k,
   \quad
   Q = \tfrac{1}{4}\, \alpha_k \alpha_k,
   \quad
   \sin \theta = \sqrt{Q}.
\end{equation}
This option reproduces the strain and vorticity increments produced by
[`ComputeFiniteStrain`](/ComputeFiniteStrain.md) when its
`decomposition_method = TaylorExpansion`.

### `rashid_eigen`

Polar-decompose $f^{-1} = r'\, u'$ and take the matrix logarithm of the two
factors:
\begin{equation}
   \Delta d_{ij} = -\log u', \quad
   \Delta w_{ij} = -\log r'.
\end{equation}
This is the "exact" $-\log f^{-1}$, accurate to machine precision for any
deformation.  It reproduces the strain and vorticity increments produced by
[`ComputeFiniteStrain`](/ComputeFiniteStrain.md) when its
`decomposition_method = EigenSolution`.

!bibtex bibliography
