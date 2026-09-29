# SigmoidTrendWeibullAux

!syntax description /AuxKernels/SigmoidTrendWeibullAux

## Description

`SigmoidTrendWeibullAux` sets an auxiliary variable to a spatially correlated Weibull random field,
like [KLWeibullAux](KLWeibullAux.md), but with a scale that varies with the distance from a line
segment. This represents, for example, a material that is on average weaker near the surface of
a specimen than along its centerline. The shape $k$, set by
[!param](/AuxKernels/SigmoidTrendWeibullAux/shape), is constant.

The segment $\gamma$ runs from $\mathbf{x}_1$, set by
[!param](/AuxKernels/SigmoidTrendWeibullAux/start_point), to $\mathbf{x}_2$, set by
[!param](/AuxKernels/SigmoidTrendWeibullAux/end_point). Both points have three components, and the
unused components should be zero in lower dimensions. The shortest distance from a point
$\mathbf{p}$ to the segment is

\begin{equation}
d(\mathbf{p}) =
\begin{cases}
\|\mathbf{p} - \mathbf{x}_1\|, & (\mathbf{x}_2 - \mathbf{x}_1)\cdot(\mathbf{p} - \mathbf{x}_1) \le 0, \\
\|\mathbf{p} - \mathbf{x}_2\|, & (\mathbf{x}_2 - \mathbf{x}_1)\cdot(\mathbf{p} - \mathbf{x}_2) \ge 0, \\
\dfrac{\|(\mathbf{p} - \mathbf{x}_1)\times(\mathbf{p} - \mathbf{x}_2)\|}{\|\mathbf{x}_2 - \mathbf{x}_1\|}, & \text{otherwise}.
\end{cases}
\end{equation}

The scale follows a logistic transition from $\lambda_{\max}$ on the segment to $\lambda_{\min}$
far from it,

\begin{equation}
\lambda(\mathbf{p}) = \lambda_{\min} + \left( \lambda_{\max} - \lambda_{\min} \right)
  \frac{g\left(d(\mathbf{p})\right)}{g(0)},
\qquad
g(d) = \frac{1}{1 + \exp \left( s \left( d - d_0 \right) \right)},
\end{equation}

where $\lambda_{\max}$ is [!param](/AuxKernels/SigmoidTrendWeibullAux/scale_max), $\lambda_{\min}$
is [!param](/AuxKernels/SigmoidTrendWeibullAux/scale_min), $d_0 \ge 0$ is
[!param](/AuxKernels/SigmoidTrendWeibullAux/midpoint_of_sigmoid), the distance at which the
logistic function $g$ is 1/2 and the transition is steepest, and $s > 0$ is
[!param](/AuxKernels/SigmoidTrendWeibullAux/slope_at_midpoint), which controls how sharp the
transition is. Dividing by $g(0)$ makes the scale exactly $\lambda_{\max}$ on the segment for any
$d_0$ and $s$. Both scales must be positive. The sampled field is

\begin{equation}
W(\mathbf{p}) = \lambda(\mathbf{p}) \left[ -\ln \left( 1 - U(\mathbf{p}) \right) \right]^{1/k},
\end{equation}

where $U(\mathbf{p})$ is the correlated uniform field obtained through a Gaussian copula from the
[KLExpansionUserObject](KLExpansionUserObject.md) named in
[!param](/AuxKernels/SigmoidTrendWeibullAux/kl_user_object). See [random_fields.md] for the theory.
[!ref](fig-trend) shows the effect of the transition steepness.

!media stochastic_tools/random_fields/cap_weibull.png
       style=width:100%;
       id=fig-trend
       caption=Weibull random fields on the unit square with a scale that trends from
       $\lambda_{\max} = 10$ on the vertical centerline to $\lambda_{\min} = 3$, with $k = 8$,
       $d_0 = 0.1$, and a steepness $s$ of (a) 1, (b) 5, and (c) 10. The Gaussian field has a
       squared-exponential covariance of unit variance and a length scale of 0.1 in both directions.

## Example Input Syntax

In this example, the scale decreases from 10 on the vertical centerline $x = 0.5$ of the unit
square to 3 away from it, with the transition centered at a distance of 0.1.

!listing modules/stochastic_tools/test/tests/userobjects/kl_expansion/weibull_2d.i
         block=AuxKernels/weibull_trend

!syntax parameters /AuxKernels/SigmoidTrendWeibullAux

!syntax inputs /AuxKernels/SigmoidTrendWeibullAux

!syntax children /AuxKernels/SigmoidTrendWeibullAux
