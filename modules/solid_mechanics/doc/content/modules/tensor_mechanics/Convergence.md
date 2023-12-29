# Setting Appropriate Convergence Criteria

It is important to set appropriate convergence criteria.  If the tolerances are too tight, models
will never converge.  If the tolerances are too weak, models will yield incorrect answers.

## Global Nonlinear Convergence Criteria

Regardless of whether plasticity or only elasticity is used, MOOSE needs to know what you mean by
"converged".  You must place a tolerance on the nonlinear residual.  In TensorMechanics this comes
from integrating $\nabla \sigma$ over an element, and for an element of side length $L$ the integral
cannot be less than
\begin{equation}
R > 10^{-P}L^{d-2}\sigma
\end{equation}
where $P$ is the precision of MOOSE (15 for double precision), $d$ is the dimensionality of the
problem (TensorMechanics currently only works for $d=3$), and $\sigma$ is the rough size of stress.

For example, for J2 plasticity, $\sigma$ will be roughly the yield strength of the material, say
$10^6$.  For a element of size 0.1, this yields $R \sim 10^{-15}\times 0.1 \times 10^{6} = 10^{-10}$.

However, you should set your nonlinear residual tolerance +substantially larger than this
estimate+.  This is because error is likely to be spread over the entire mesh, not just one element,
so $R$ should be multiplied by the number of elements, and also that setting $R$ this small is risky
because it relies on maintaining full precision throughout all computations.  Instead, identify the
crucial region of interest.  Denote its volume by $V$.  Usually this will be less than the entire
mesh.  Then identify the error in $\nabla\sigma$ that you are willing to accept.  Denote this by
$E_{\nabla\sigma}$.  It will depend on the typical size of $\sigma$, and the length scale in the
problem.  Then set $R = VE_{\nabla\sigma}$.


## Convergence Criteria for UserObject Based Plasticity

There are three types of convergence criteria that need to be set when using plasticity:

1. tolerance(s) on the yield function(s)
2. tolerance on the plastic strain
3. tolerance(s) on the internal parameter(s)

These are somewhat interconnected.

### Tolerance(s) on the yield function(s)

Firstly, let us explore lower bounds on the tolerance for the yield function(s).  Consider the stress
tensor, $\sigma$.  I want to calculate $E_{\sigma}$: any changes of $\sigma$ smaller than
$E_{\sigma}$ will be unnoticeable due to precision loss.

Imagine that $\sigma$ has $P$ digits of precision.  For standard MOOSE with double precision, $P=15$.
In models containing plasticity, the stress will often reside on the yield surface(s).  This gives an
estimate of the magnitude of $\sigma$.  For instance, with Mohr-Coulomb plasticity, stresses will be
of order $C\cot(\phi)$, where $C$ is the cohesion, and $\phi$ is the friction angle, so $C\cot(\phi)$
is at the apex of the hexagonal pyramid.  Denote this magnitude of $\sigma$ by $f$.  For complicated
multi-surface plasticity models, $f$ may be hard to estimate, but only order-of-magnitude
calculations are appropriate here.  Therefore,
\begin{equation}
E_{\sigma} = 10^{-P}f
\end{equation}
For example, with $P=15$, and Mohr-Coulomb plasticity with $C=10^{6}$ and friction angle 30deg,
$E_{\sigma} = 10^{-9}$.  Changes of $\sigma$ smaller than this value will be unnoticeable due to
precision loss.

There is also another lower bound.  This comes from evaluating the trial stress and returned stress
using the strain: $\sigma \sim C\epsilon$, where $C$ is the elasticity tensor, and $\epsilon$ is the
strain.  This means that
\begin{equation}
E_{\sigma} = 10^{-P}C\epsilon
\end{equation}
For example, suppose strains of order $10^{-1}$ are applied, and the elasticity tensor is order
$10^{10}$.  This means trial stresses will be of order $10^{9}$, so with $P=15$, we get $E_{\sigma} =
10^{-6}$.

In conclusion, changes of $\sigma$ smaller than $E_{\sigma}$ will be unnoticeable due to precision
loss, where $E_{\sigma}$ is
\begin{equation}
E_{\sigma} = 10^{-P}\max(f, C\epsilon)
\end{equation}

Knowing this allows straightforward estimation of the +lowest possible+ tolerance on the yield
functions.  +However, it is unwise to use this tolerance!+ This is because roundoff errors can accrue
within the internal workings of the plasticity algorithms.  For instance, calculating eigenvalues of
$\sigma$ might effectively reduce $P$ by 1.

More commonly, the tolerance on stress is estimated by the user by considering what is physically
reasonable.  Eg, a stress change of 100Pa might be rather inconsequential, and this might translate
into a tolerance on $f$ of 100Pa.  Provided this 100Pa is not close to $E_{\sigma}$, the plasticity
algorithms should converge.

!alert note prefix=False
Choose the tolerance on $f$ to be $10^5$ smaller than the yield stress, cohesion, tensile strength,
etc.


### Tolerances on the plastic strain and internal constraints

Similar arguments can be made for the plastic strain and internal constraints.

!alert note prefix=False
Choose the tolerance to be $10^{-5}\epsilon$ to $10^{-3}\epsilon$, where $\epsilon$ are the typical strains
encountered in the model
