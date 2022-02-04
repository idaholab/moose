# WeightedTransition

This class is used for objects that perform smooth transitions between two
functions of one variable. Denoting the transition begin and end points by
$x_1$ and $x_2$, respectively, and the "left" and "right" functions by $f_1(x)$
and $f_2(x)$, respectively, the transition function $f(x)$ is the following:
\begin{equation}
  f(x) = w(x) f_1(x) + (1 - w(x)) f_2(x) \,,
\end{equation}
where the weight $w_x(x)$ is between 0 and 1 and is computed with a cosine
function:
\begin{equation}
  w(x) = \left\{\begin{array}{l l}
    1 & x \le x_1\\
    0 & x \ge x_2\\
    \frac{1}{2}\left(\cos\left(\frac{\pi}{x_2 - x_1}(x - x_1)\right) + 1\right) & \text{otherwise}\\
    \end{array}\right. \,.
\end{equation}
The transitioned function $f(x)$ has the following desirable properties:

- $f(x)$ is continuous throughout the interval $[x_1, x_2]$ (including at the end points),
  so long as $f_1(x)$ and $f_2(x)$ are also continuous on this interval.
- $f'(x)$ is continuous throughout the interval $[x_1, x_2]$ (including at the end points),
  so long as $f_1'(x)$ and $f_2'(x)$ are also continuous on this interval.

An example of the transition is illustrated in [WeightedTransition_discontinuous],
where is transition is created in the region $x\in(1,5)$ at a discontinuity:

!media thermal_hydraulics/misc/WeightedTransition_discontinuous.png
       id=WeightedTransition_discontinuous
       caption=Weighted transition at a discontinuity
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

Note that the transition can exit the bounds of the non-transitioned piecewise
function, which can be undesirable. This is illustrated in [WeightedTransition_continuous],
where compares this weighted approach with the approach of [CubicTransition.md]:

!media thermal_hydraulics/misc/WeightedTransition_continuous.png
       id=WeightedTransition_continuous
       caption=Transition at the intersection of two functions
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

Thus when the transition occurs at the intersection of two functions, it may be
advantageous to use [CubicTransition.md] instead of `WeightedTransition`,
whereas transitions at a discontinuity may be handled well with
`WeightedTransition`.
