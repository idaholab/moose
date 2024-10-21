# CappedWeakPlaneStressUpdate

!syntax description /Materials/CappedWeakPlaneStressUpdate

## Theory id=theory

Weak-plane plasticity is designed to simulate a layered material.
Each layer can slip over adjacent layers, and be separated from those
adjacent layers.  An example of particular interest is
stratified rocks, which consist of large sheets of fairly strong rock
separated by weak and very thin joints.  Upon application of stress,
the joints can fail, either by slipping or separating.  The idea is to
use one finite element that potentially contains many layers, and
prescribe ``weak plane plasticity'' for that finite element, so that
it can fail by joint separation and joint slipping.

Denote the normal to the layers by $z$, and the tangential directions
by $x$ and $y$.  It is convenient to introduce two new stress variables in
terms of the stress tensor $\sigma$:

\begin{equation}
p = \sigma_{zz} \ \ \ \text{and}\ \ \ q = \sqrt{\sigma_{xz}^{2} +
  \sigma_{yz}^{2}} \ .
\label{eqn.defn.p.q}
\end{equation}

In standard elasticity, the stress tensor is symmetric, so an
equivalent definition of $q$ is
$q=\sqrt{\frac{1}{2}(\sigma_{xz}+\sigma_{zx})^{2} + \frac{1}{2}(\sigma_{yz}+\sigma_{zy})^{2}}$, however the symmetrization is deliberately not written in
[eqn.defn.p.q] and below so that the equations also hold for
the Cosserat case (see [CappedWeakPlaneCosseratStressUpdate.md]).

The joint slipping is assumed to be governed by a
Drucker-Prager type of plasticity with a cohesion $C$, and friction
angle $\phi$:

\begin{equation}
f_{0} = q + p\tan\phi - C \ .
\end{equation}

The parameter $C$ and $\phi$ may be constants, or they may harden or
soften (more on this later).

Joints may also open, and this type of failure is assumed to be
governed by a tensile failure yield function:

\begin{equation}
f_{1} = p - S_{T} \ ,
\end{equation}

where $S_{T}$ is the tensile strength, which may be constant or harden
or soften.

Joints may also close, and this type of failure is assumed to be
governed by a compressive failure yield function:

\begin{equation}
f_{2} = - p - S_{C} \ ,
\end{equation}

where $S_{C}$ is the compressive strength (a positive quantity), which
may be constant or harden or soften.

The yield functions $f_{1}$ and $f_{2}$ place ``caps'' on the shear
yield function $f_{0}$.
The combined yield function is simply

\begin{equation}
f = \max(f_{0}, f_{1}, f_{2}) \ ,
\label{one.yf.eqn}
\end{equation}

which defines the admissible domain where all yield functions are
non-positive, and the inadmissible domain where at least one yield
function is positive.


One of the features of this plasticity is the
ability to model cyclic behavior. For instance, the compressive
strength may be initially very high. However, after tensile failure,
the compressive strength can soften to zero in order to model the fact
that the material now contains open joints which cannot support any
compressive load. If the material then fails in compression (eg,
because it gets squashed) and the joints close then the compressive
strength can be made high again.

## Flow rules and hardening

This plasticity is non-associative.  Define the dilation angle $\psi$,
which may be constant, or harden or soften.  The shear flow potential
is

\begin{equation}
g_{0} = q + p\tan\psi .
\end{equation}

The tensile flow potential is

\begin{equation}
g_{1} = p \ ,
\end{equation}

and the compressive flow potential is

\begin{equation}
g_{2} = - p \ .
\end{equation}

The overall flow potential is

\begin{equation}
g = \left\{
\begin{array}{ll}
g_{0} & \text{ if } f = f_{0} \\
g_{1} & \text{ if } f = f_{1} \\
g_{2} & \text{ if } f = f_{2}\ .
\end{array}
\label{one.g.eqn}
\right.
\end{equation}

Obviously there are problems here where $g$ is not defined properly at
the corners where $f_{0}=f_{1}$ and $f_{0}=f_{2}$ (or even
$f_{0}=f_{1}=f_{2}$).  This is resolved by using smoothing (more on
this later).

This plasticity model contains two internal parameters, denote by
$i_{0}$ and $i_{1}$.   It is assumed
that

\begin{equation}
\begin{split}
C & = C(i_{0}) \ , \\
\phi & = \phi(i_{0}) \ , \\
\psi & = \psi(i_{0}) \ , \\
S_{T} & = S_{T}(i_{1}) \ , \\
S_{C} & = S_{C}(i_{1}) \ . \\
\end{split}
\end{equation}

That is, $i_{0}$ can be thought of as the ``shear'' internal parameter,
while $i_{1}$ is the ``tensile'' internal parameter.

To complete the definition of this plasticity model, the increments of
$i_{0}$ and $i_{1}$ during the return-map process must be defined.
The return-map process involves being provided with a trial stress
$\sigma^{\mathrm{trial}}$ and an existing value of the internal
parameters $i^{\mathrm{old}}$, and finding a ``returned'' stress,
$\sigma$, and internal parameters, $i$, that satisfy

\begin{equation}
\begin{split}
0 & = f(\sigma, i) \ . \\\label{f.zero.return}
\sigma & = \sigma^{\mathrm{trial}} - E\gamma \frac{\partial g}{\partial\sigma} \ , \\
\end{split}
\end{equation}

where $E$ is the elasticity tensor, and $\gamma$ is a ``plastic
multiplier'', that must be positive.  The former expresses that the
stress must be admissible, while the latter is called the ``normality
condition''.  Loosely speaking, the returned stress lies at a position
on the yield surface where the normal points to the trial stress
(actually $E$ and $\partial g/\partial\sigma$ must be used to define
the ``normal direction'').

Let us express the normality condition in $(p, q)$ space.  The $zz$
component is easy:

\begin{equation}
p = \sigma_{zz} = \sigma_{zz}^{\mathrm{trial}} - E_{zzij}\gamma
\frac{\partial g}{\partial \sigma_{ij}} = \sigma_{zz}^{\mathrm{trial}} - E_{zzzz}\gamma
\frac{\partial g}{\partial p}  \ ,
\label{p.norm.eqn}
\end{equation}

where the last equality holds by assumption (see full list of
assumptions below).  The $xz$ and $yz$ components are similar:

\begin{equation}
\sigma_{xz} = \sigma_{xz}^{\mathrm{trial}} - E_{xzxz}\gamma
\frac{\partial g}{\partial q}\frac{\partial q}{\partial \sigma_{xz}}
\ . \label{eqn.xz.flow}
\end{equation}

Another assumption has been made about $E$.  The final term is

\begin{equation}
\frac{\partial q}{\partial\sigma_{xz}} = \frac{\sigma_{xz}}{q} \ .
\end{equation}

This means that [eqn.xz.flow] can be re-written

\begin{equation}
\sigma_{xz}^{2} \left( 1 + E_{xzxz}\gamma \frac{\partial g}{\partial
  q} \frac{1}{q} \right)^{2} =
\left(\sigma^{\mathrm{trial}}_{xz}\right)^{2} \ .
\end{equation}

A similar equation holds for the $yz$ component, and these can be
summed and rearranged to yield

\begin{equation}
q = q^{\mathrm{trial}} - E_{xzxz}\gamma \frac{\partial g}{\partial q}
\ . \label{q.norm.eqn}
\end{equation}

[f.zero.return], [p.norm.eqn]
and~[q.norm.eqn] are the three conditions that need to be
satisfied, and the three variables to be found are $p$, $q$ and
$\gamma$.

Consider the case of returning to the shear yield surface.
Since $\partial g/\partial p =
\tan\psi$ and $\partial g/\partial q = 1$ for this flow, the
return-map process must solve the following system of equations

\begin{equation}
\begin{split}
0 & = q + p\tan\phi - C \ , \\
p & = p^{\mathrm{trial}} - E_{zzzz}\gamma\tan\psi \ , \\
q & = q^{\mathrm{trial}} - E_{xzxz}\gamma \ . \\
\end{split}
\end{equation}

The solution satisfied $p^{\mathrm{trial}} - p =
E_{zzzz}\gamma\tan\psi$ and $q^{\mathrm{trial}} - q = E_{xzxz}\gamma$.

Now consider the case of returning to the tensile yield surface.  The
equations are

\begin{equation}
\begin{split}
0 & = p - S_{T} \ , \\
p & = p^{\mathrm{trial}} - E_{zzzz}\gamma \ , \\
q & = q^{\mathrm{trial}} \ . \\
\end{split}
\end{equation}

Comparing these two types of return, it is obvious that
$q^{\mathrm{trial}} - q$ quantifies the amount of shear failure.
Therefore, the following definitions are used in this plasticity model

\begin{equation}
\begin{split}
i_{0} & = i_{0}^{\mathrm{old}} + \frac{q^{\mathrm{trial}} - q}{E_{xzxz}} \ , \\
i_{1} & = i_{1}^{\mathrm{old}} + \frac{p^{\mathrm{trial}} - p}{E_{zzzz}} - \frac{(q^{\mathrm{trial}} - q)\tan\psi}{E_{xzxz}} \ . \\
\end{split}
\end{equation}

The final term ensures that $i_{1}$ does not increase during pure
shear failure.  The scaling by $E$ ensures that these internal
parameters are dimensionless.

In summary, this plasticity model is defined by the yield function of
[one.yf.eqn], the flow potential of
[one.g.eqn], and the following return-map problem.

### Return-map problem id=rmap

At any given MOOSE timestep, given the
old stress $\sigma_{ij}^{\mathrm{old}}$, and a total strain increment
$\delta \epsilon$ (that comes from the nonlinear solver proposing
displacements) the trial stress is

\begin{equation}
\sigma_{ij}^{\mathrm{trial}} = \sigma_{ij}^{\mathrm{old}} +
E_{ijkl}\delta\epsilon_{kl} \ ,
\end{equation}

This gives $p^{\mathrm{trial}}$ and $q^{\mathrm{trial}}$.  If
$p^{\mathrm{trial}}$, $q^{\mathrm{trial}}$, $i_{0}^{\mathrm{old}}$ and
$i_{1}^{\mathrm{old}}$ are such that $f(p^{\mathrm{trial}}, q^{\mathrm{trial}},
i^{\mathrm{old}}) > 0$, the return-map problem is: find $p$, $q$,
$\gamma$, $i_{0}$ and $i_{1}$ such that

\begin{equation}
\begin{split}
0 & = f(p, q, i) \ ,  \\
p & = p^{\mathrm{trial}} - E_{zzzz}\gamma \frac{\partial g}{\partial p} \ ,  \\
q & = q^{\mathrm{trial}} - E_{xzxz}\gamma \frac{\partial g}{\partial q} \ ,  \\
i_{0} & = i_{0}^{\mathrm{old}} + \frac{q^{\mathrm{trial}} - q}{E_{xzxz}} \ ,  \\
i_{1} & = i_{1}^{\mathrm{old}} + \frac{p^{\mathrm{trial}} - p}{E_{zzzz}} - \frac{(q^{\mathrm{trial}} - q)\tan\psi}{E_{xzxz}} \ . \\
\label{rmp.eqn}
\end{split}
\end{equation}

The latter two equations are assumed to hold in the smoothed situation
(discussed below) too, and note that $\psi = \psi(i_{0})$, so the
these two equations are not completely trivial.

After the return-map problem has been solved, the stress components
are $\sigma_{ij} = \sigma_{ij}^{\mathrm{trial}}$, except for the following

\begin{equation}
\begin{split}
\sigma_{xx} & = \sigma_{xx}^{\mathrm{trial}} - E_{zzxx}\gamma\frac{\partial g}{\partial p} \ , \\
\sigma_{yy} & = \sigma_{yy}^{\mathrm{trial}} - E_{zzyy}\gamma\frac{\partial g}{\partial p} \ , \\
\sigma_{zz} & = p \ , \\
\sigma_{zx} & = \sigma_{zx}^{\mathrm{trial}} q / q^{\mathrm{trial}} \ , \\
\sigma_{xz} & = \sigma_{xz}^{\mathrm{trial}} q / q^{\mathrm{trial}} \ , \\
\sigma_{zy} & = \sigma_{zy}^{\mathrm{trial}} q / q^{\mathrm{trial}} \ , \\
\sigma_{yz} & = \sigma_{yz}^{\mathrm{trial}} q / q^{\mathrm{trial}} \ . \\
\end{split}
\end{equation}

The plastic strain is

\begin{equation}
\epsilon_{ij}^{\mathrm{plastic}} = \epsilon_{ij}^{\mathrm{plastic, old}} +
\delta\epsilon_{ij} + E_{ijkl}^{-1}(\sigma_{kl}^{\mathrm{old}} -
\sigma_{kl}) = \epsilon_{ij}^{\mathrm{plastic, old}} + E_{ijkl}\gamma
\frac{\partial g}{\partial \sigma_{kl}}\ .
\end{equation}

The elastic strain is

\begin{equation}
\epsilon_{ij}^{\mathrm{elastic}} = \epsilon_{ij}^{\mathrm{elastic, old}} +
\delta\epsilon_{ij} - \epsilon_{ij}^{\mathrm{plastic}} +
\epsilon_{ij}^{\mathrm{plastic, old}} \ .
\end{equation}


## Yield Smoothing

The shear yield function, $f_{0}$, describes a cone in $(\sigma_{yz},
\sigma_{xz}, \sigma_{zz})$ space.  The cone's tip is problematic for the
return-map process (the derivative is not defined there) and there are
two main ways of getting around this.  Firstly, a multi-surface
technique can be used to define the return-map process.  Secondly, the
cone's tip can be smoothed.  This plasticity model uses the second
technique.  The yield function is defined to be

\begin{equation}
f_{0} = \sqrt{q^{2} + s_{t}^{2}} + p\tan\phi - C \ ,
\end{equation}

and the flow potential is

\begin{equation}
g_{0} = \sqrt{q^{2} + s_{t}^{2}} + p\tan\psi \ .
\end{equation}

The vertices where the shear yield surface meets the tensile and
compressive yield surfaces also need to be handled.  Smoothing is also
used here.  This uses a new type of smoothing.  For
the case at hand only two yield surfaces and flow potentials need to
be smoothed (there are no points where three or more yield surfaces
get close to each other) and only in 2D space, and a single parameter
$s$ can be used.  The parameter $s$ has the units of stress.  At any
point $(p, q, i)$ order the 3 yield function values, and denote the
largest by $A$, the second largest by $B$ and the smallest by $C$:

\begin{equation}
A\geq B\geq C
\end{equation}

Then the single, smoothed yield function is defined to be

\begin{equation}
f = \left\{
\begin{array}{ll}
A & \ \ \ \text{if}\ \ A\geq B+s \\
\frac{A+B+s}{2} -
\frac{s}{\pi}\cos\left(\frac{(B-A)\pi}{2s}\right) \ .
\end{array}
\right.
\end{equation}

The derivative of the flow potential is smoothed similarly.

## Constraints and assumptions concerning parameters id=assumptions

The friction angle and cohesion should be positive, and the dilation
angle should be non-negative.  Furthermore, the MOOSE user must ensure that
\begin{equation}
\psi \leq \phi \ .
\end{equation}
These conditions should be satisfied for all values of the internal
parameter $i_{0}$.  MOOSE checks that these conditions hold for
$i_{0}=0$ only.

The tensile and compressive strength must satisfy
\begin{equation}
S_{T} \geq -S_{C} \ ,
\end{equation}
otherwise the ``caps'' are swapped and the assumption of a convex
yield surface is violated.  MOOSE checks this condition holds for
$i_{1}=0$ only: the MOOSE user must ensure that it actually holds for
all values of the internal parameter

The smoothing parameter $s$ must be chosen carefully.  At no time should
the tensile cap mix with the compressive cap via smoothing, otherwise
this typically means that no stress is admissible and MOOSE will never
converge.  For instance, if $S_{T}=1=S_{C}$, then a smoothing
parameter of 0.1 is fine, but a smoothing parameter $\geq 2$ will
cause mixing of tension with compression.  The MOOSE user must ensure
that this holds for all values of the internal parameters.

The tip-smoothing parameter $s_{t}$ is important, even if the tensile cap
completely chops off the shear-cone's tip.  This is because MOOSE can
explore regions of parameter space where the cone's tip is exposed.

It is vital that the smoothing parameters $s$ and $s_{t}$ are chosen so
that the yield surface is not wildly varying around $q=0$, otherwise
poor convergence of the return-map process will occur.

It is assumed that the elasticity tensor has the following symmetries:

\begin{equation}
E_{ijkl} = E_{jikl} = E_{ijlk} = E_{klij} \ ,
\label{eqn.elas.symms}
\end{equation}

and that

\begin{equation}
0 = E_{zzij} \ \ \ \text{if}\ \ \ i\neq j \ ,
\end{equation}

and that

\begin{equation}
E_{xzxz} = E_{yzyz} \ ,
\end{equation}

and that

\begin{equation}
0 = E_{xzij} \ \ \text{ unless } (i, j) = (z, x) \ \ \text{ or } (i,
j) = (x, z) \ .
\label{eqn.elas.xz.con}
\end{equation}

These are quite standard conditions that hold for all non-Cosserat
materials to our knowledge.

## Technical discussions

### Unknowns and the convergence criterion

The return-map problem [rmp.eqn] is solved as a $3\times 3$
system consisting of the first 3 equations, and substituting the fourth and
fifth equations wherever needed.  The three unknowns are $p$, $q$ and
$\gamma_{E}=\gamma E_{zzzz}$, which all have the same units.  Convergence
is deemed to be achieved when the sum of squares of the residuals of
these 3 equations is less than a user-defined tolerance.

### Iterative procedure and initial guesses

A Newton-Raphson process is used, along with a cubic line-search.  The
process may be initialized with the solution that is correct for
perfect plasticity (no hardening) and no smoothing, if the user
desires.  Smoothing adds nonlinearities, so this initial guess will
not always be the exact answer. For hardening, it is not
always advantageous to initialize the Newton-Raphson process in this
way, as the yield surfaces can move dramatically during the return
process.

### Sub-stepping the strain increments

Because of the difficulties encountered during the Newton-Raphson
process during rapidly hardening/softening moduli, it is possible to
subdivide the applied strain increment, $\delta\epsilon$, into smaller
sub-steps, and do multiple return-map processes.  The final returned configuration will then
be dependent on the number of sub-steps.  While this is simply
illustrating the non-uniqueness of plasticity problems, in my
experience it does adversely affect MOOSE's nonlinear convergence as
some Residual calculations will take more sub-steps than other Residual
calculations: in effect this is reducing the accuracy of the Jacobian.

## The consistent tangent operator

MOOSE's Jacobian depends on the derivative

\begin{equation}
H_{ijkl} = \frac{\delta\sigma_{ij}}{\delta \epsilon_{kl}} \ .
\end{equation}

The quantity $H$ is called the consistent tangent operator.  For pure
elasticity it is simply the elastic tensor, $E$, but it is more
complicated for plasticity.  Note that a small $\delta\epsilon_{kl}$
simply changes $\delta\sigma^{\mathrm{trial}}$, so $H$ is capturing the
change of the returned stress ($\delta\sigma$) with respect to a
change in the trial stress ($\delta\sigma^{\mathrm{trial}}$).  In
$(p,q)$ language, we need to the sx derivatives

\begin{equation}
\frac{\delta (p, q, \gamma)}{\delta (p^{\mathrm{trial}},
  q^{\mathrm{trial}})} \ .
\end{equation}

The algebra is extremely tedious, but it is fairly easy for the
computer.  The MOOSE code contains two implementations of the
consistent tangent operator.  One is valid for any general $(p, q)$
model, while the other is specialized to the weak-plane case.

### General consistent tangent operator

The return-map algorithm provides

\begin{equation}
\sigma_{ij} = \sigma_{ij}^{\mathrm{trial}} - E_{ijmn}\gamma
\frac{\partial g}{\partial \sigma_{mn}} \ .
\end{equation}

Since $\sigma^{\mathrm{trial}} = E\epsilon$, the consistent tangent
operator is

\begin{equation}
\begin{split}
H_{ijkl} &= E_{ijkl} - E_{ijmn} E_{pqkl} \frac{\partial}{\partial \sigma_{pq}^{\mathrm{trial}}} \gamma \frac{\partial g}{\partial \sigma_{mn}} \\
&= E_{ijkl} - E_{ijmn} E_{pqkl} \left( \frac{\partial p^{\mathrm{trial}}}{\partial \sigma_{pq}^{\mathrm{trial}}} \frac{\partial}{\partial p^{\mathrm{trial}}} + \frac{\partial q^{\mathrm{trial}}}{\partial \sigma_{pq}^{\mathrm{trial}}} \frac{\partial}{\partial q^{\mathrm{trial}}} \right) \gamma \left( \frac{\partial g}{\partial p} \frac{\partial p}{\partial \sigma_{mn}} + \frac{\partial g}{\partial q} \frac{\partial q}{\partial \sigma_{mn}} \right)
\end{split}
\end{equation}

However, note that

\begin{equation}
\frac{\partial}{\partial p^{\mathrm{trial}}} \left( p -
p^{\mathrm{trial}} + \gamma E_{pp}\frac{\partial g}{\partial p}
\right) = 0 \ ,
\end{equation}

because the return-map algorithm guarantees that the expression inside
parentheses is zero.  Therefore

\begin{equation}
\frac{\partial}{\partial p^{\mathrm{trial}}}\gamma\frac{\partial
  g}{\partial p} = \frac{1}{E_{pp}} \left( 1- \frac{\partial
  p}{\partial p^{\mathrm{trial}}} \right) \ .
\end{equation}

A similar expression holds for three other cases.  There are still
terms that involve derivatives of $\partial p/\partial \sigma_{mn}$
  and $\partial q/\partial\sigma_{mn}$, but these may be separated off
  as seen below.

The consistent tangent operator may therefore be written as

\begin{equation}
\begin{split}
H_{ijkl} & = E_{ijkl} - E_{ijmn}E_{pqkl} \left\{ \frac{\partial p^{\mathrm{trial}}}{\partial\sigma_{pq}^{\mathrm{trial}}} \frac{1}{E_{pp}}\left(1 - \frac{\partial p}{\partial p^{\mathrm{trial}}} \right) \frac{\partial p}{\partial\sigma_{mn}} \right.  \\
& \left. \frac{\partial q^{\mathrm{trial}}}{\partial\sigma_{pq}^{\mathrm{trial}}} \frac{1}{E_{pp}}\left( - \frac{\partial p}{\partial q^{\mathrm{trial}}} \right) \frac{\partial p}{\partial\sigma_{mn}} + \frac{\partial p^{\mathrm{trial}}}{\partial\sigma_{pq}^{\mathrm{trial}}}\ \frac{1}{E_{qq}}\left(- \frac{\partial q}{\partial p^{\mathrm{trial}}} \right) \frac{\partial q}{\partial\sigma_{mn}} + \frac{\partial q^{\mathrm{trial}}}{\partial\sigma_{pq}^{\mathrm{trial}}} \frac{1}{E_{qq}}\left(1 - \frac{\partial q}{\partial q^{\mathrm{trial}}} \right) \frac{\partial q}{\partial\sigma_{mn}} \right\}  \\
& \frac{\partial \sigma_{ab}}{\partial\epsilon_{kl}} E_{ijmn} \gamma \left( \frac{\partial g}{\partial p}\frac{\partial^{2} p}{\partial\sigma_{mn}\partial\sigma_{ab}} + \frac{\partial g}{\partial q}\frac{\partial^{2} q}{\partial\sigma_{mn}\partial\sigma_{ab}} \right) \ . \\
\end{split}
\end{equation}

All terms but the final line have already been computed during the
return-map process.  The final line may be brought to the right-hand
side (since $H_{ijkl} = \partial\sigma_{ij}/\partial\epsilon_{kl}$)
and the resulting expression multiplied inverse $H$'s coefficient to
finally yield $H$.  This inversion, and all the multiplication of
rank-four tensors may be computationally expensive, so a cheaper (but
more lengthy looking) version is derived below for the capped
weak-plane case.

### Specialization to the weak-plane case

The return-map equations [rmp.eqn] are obtaining $(p, q)$
given the trial variables.  Finding $H$ is really just re-solving
these equations for a slightly changed trial variable.  Denote

\begin{equation}
\left( \begin{array}{l} R_{0} \\ R_{1} \\ R_{2} \end{array} \right)
=
\left( \begin{array}{l} -f \\ -p + p^{\mathrm{trial}} - E_{zzzz}\gamma \frac{\partial g}{\partial
  p} \\ -q + q^{\mathrm{trial}} - E_{xzxz}\gamma \frac{\partial g}{\partial
  q} \end{array} \right) \ .
\end{equation}

Then

\begin{equation}
\begin{split}
\frac{\partial R_{0}}{\partial p^{\mathrm{trial}}} & = -\frac{\partial f}{\partial i_{1}} \frac{\partial i_{i}}{\partial p^{\mathrm{trial}}}  \ , \\
\frac{\partial R_{1}}{\partial p^{\mathrm{trial}}} & = 1 - E_{zzzz}\gamma \frac{\partial^{2}g}{\partial p\partial i_{i}}\frac{\partial i_{i}}{\partial p^{\mathrm{trial}}} \ , \\
\frac{\partial R_{2}}{\partial p^{\mathrm{trial}}} & = -E_{xzxz}\gamma \frac{\partial^{2}g}{\partial q\partial i_{i}}\frac{\partial i_{i}}{\partial p^{\mathrm{trial}}} \\
\end{split}
\end{equation}

In these equations

\begin{equation}
\frac{\partial i_{1}}{\partial p^{\mathrm{trial}}} =
\frac{1}{E_{zzzz}} \ ,
\end{equation}

which comes from [rmp.eqn].  The derivatives with respect to
$q^{\mathrm{trial}}$ are similar but more lengthy due to both $i_{0}$
and $i_{1}$ being dependent on $q^{\mathrm{trial}}$.  The system to
solve is

\begin{equation}
\left( \begin{array}{ccc}
\frac{\partial R_{0}}{\partial \gamma} & \frac{\partial R_{0}}{\partial p} & \frac{\partial R_{0}}{\partial q} \\
\frac{\partial R_{1}}{\partial \gamma} & \frac{\partial R_{1}}{\partial p} & \frac{\partial R_{1}}{\partial q} \\
\frac{\partial R_{2}}{\partial \gamma} & \frac{\partial
  R_{2}}{\partial p} & \frac{\partial R_{2}}{\partial q}
\end{array} \right)
\left( \begin{array}{c}
\delta \gamma/\delta p^{\mathrm{trial}} \\
\delta p/\delta p^{\mathrm{trial}} \\
\delta q/\delta p^{\mathrm{trial}}
\end{array} \right) =
\left( \begin{array}{c}
\partial R_{0}/\delta p^{\mathrm{trial}} \\
\partial R_{1}/\delta p^{\mathrm{trial}} \\
\partial R_{2}/\delta p^{\mathrm{trial}}
\end{array} \right)
\end{equation}

The $3\times 3$ Jacobian matrix is identical to the one used in the
Newton-Raphson process, but of course that process has completed
before calculation of the consistent tangent operator.  A similar
system of equations gives the derivatives with respect to
$q^{\mathrm{trial}}$.

Once the six derivatives have been computed they need to be assembled
into $H$.  For instance,

\begin{equation}
\frac{\delta p^{\mathrm{trial}}}{\delta \epsilon_{ii}} = E_{zzii}
\ ,
\end{equation}

so that

\begin{equation}
H_{zzii} = \frac{\delta p}{\delta p^{\mathrm{trial}}} E_{zzii} \ ,
\end{equation}

and other more complicated expressions appear for other components, such as

\begin{equation}
\begin{split}
H_{xxii} & = E_{xxii} - E_{zzxx} E_{zzii}\left( \frac{\delta \gamma}{\delta p^{\mathrm{trial}}} \frac{\partial g}{\partial p} + \gamma \frac{\partial^{2} g}{\partial p^{2}} \frac{\delta p}{\delta p^{\mathrm{trial}}} + \gamma \frac{\partial^{2} g}{\partial p\partial i_{1}} \frac{\delta q}{\delta p^{\mathrm{trial}}} + \gamma \frac{\partial^{2} g}{\partial p\partial i_{0}} \frac{\partial i_{0}}{\partial q} \frac{\delta q}{\delta p^{\mathrm{trial}}} \right.  \\
& \ \ \ \ + \left. \gamma \frac{\partial^{2} g}{\partial p\partial i_{1}} \left( \frac{\delta i_{1}}{\delta p^{\mathrm{trial}}} + \frac{\partial i_{1}}{\partial p} \frac{\delta p}{\delta p^{\mathrm{trial}}} \frac{\partial i_{1}}{\partial q} \frac{\delta q}{\delta p^{\mathrm{trial}}} \right) \right) \ . \\
\end{split}
\end{equation}

### The consistent tangent operator and sub-stepping strain increments

One extra complication arises from the potential sub-stepping of the
applied strain increment $\delta\epsilon$.  At each sub-step,
the six derivatives must be computed.  While this may seem expensive,
in my experience it increases the accuracy of the Jacobian, and the
main computational expense is building and solving the $3\times 3$
system which is pretty quick for the computer to compared with the
entire Newton-Raphson process.

Let the $n^{\mathrm{th}}$ substep be

\begin{equation}
\delta\epsilon^{n} = \lambda_{n}\delta\epsilon \ ,
\end{equation}

with

\begin{equation}
1 = \sum_{n=1}^{N}\lambda_{n} \ ,
\end{equation}

where $N$ is the total number of substeps.  Denoting the initial
stress by $(p^{\mathrm{old}}, q^{\mathrm{old}})$, and the returned
stress at step $n-1$ by $(p_{n-1}, q_{n-1})$, of course the trial
stress at step $n$ is

\begin{equation}
(p_{n}^{\mathrm{trial}}, q_{n}^{\mathrm{trial}}) = (p_{n-1},q_{n-1}) +
  \lambda_{n} ( p^{\mathrm{trial}} - p^{\mathrm{old}},
    q^{\mathrm{trial}} - q^{\mathrm{old}}) \ .
\end{equation}

This means that

\begin{equation}
\begin{split}
\frac{\partial p_{n}}{\partial p^{\mathrm{trial}}} & = \frac{\partial p_{n}}{\partial p_{n}^{\mathrm{trial}}} \frac{\partial p_{n}^{\mathrm{trial}}}{\partial p^{\mathrm{trial}}} + \frac{\partial p_{n}}{\partial q_{n}^{\mathrm{trial}}} \frac{\partial q_{n}^{\mathrm{trial}}}{\partial p^{\mathrm{trial}}} \\
& = \frac{\partial p_{n}}{\partial p_{n}^{\mathrm{trial}}} \left(\lambda_{n} + \frac{\partial p_{n-1}}{\partial p^{\mathrm{trial}}} \right) + \frac{\partial p_{n}}{\partial q_{n}^{\mathrm{trial}}} \frac{\partial q_{n-1}}{\partial p^{\mathrm{trial}}} \ . \\
\end{split}
\end{equation}

Similar inductive equations hold for the other derivatives, and note
that $\partial p_{0}/\partial p^{\mathrm{trial}} = \partial
p^{\mathrm{old}}/\partial p^{\mathrm{trial}} = 0$.  The derivative of
$\gamma$ is slightly different: it is

\begin{equation}
\frac{\partial \gamma}{\partial p^{\mathrm{trial}}} = \sum_{n=1}^{N}
\frac{\partial \gamma_{n}}{\partial p_{n}^{\mathrm{trial}}}
\left(\lambda_{n} + \frac{\partial p_{n-1}}{\partial p^{\mathrm{trial}}}
\right)
+ \frac{\partial \gamma_{n}}{\partial q_{n}^{\mathrm{trial}}}
\frac{\partial q_{n-1}}{\partial p^{\mathrm{trial}}} \ ,
\end{equation}

and similarly for the derivative with respect to $q^{\mathrm{trial}}$.

!syntax parameters /Materials/CappedWeakPlaneStressUpdate

!syntax inputs /Materials/CappedWeakPlaneStressUpdate

!syntax children /Materials/CappedWeakPlaneStressUpdate

