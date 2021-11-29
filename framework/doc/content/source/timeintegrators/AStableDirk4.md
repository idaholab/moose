# AStableDirk4

!syntax description /Executioner/TimeIntegrator/AStableDirk4

The Butcher tableau for this method is:

!table
| $c_i$ | $a_{i1}$ | $a_{i2}$ | $a_{i3}$ | $a_{i4}$ |
| - | - | - | - | - |
| $\gamma$   | $\gamma$        | 0               | 0 |
| 1/2        | $1/2-\gamma$    | $\gamma$        | 0 |
| $1-\gamma$ | $2\gamma$        | $1-4\gamma$    | $\gamma$ |
| $b_{j}$ | $\dfrac{1}{24(1/2-\gamma)}$    | $1 - \dfrac{1}{12(1/2-\gamma)^2}$ | $\dfrac{1}{24(1/2-\gamma)^2}$ |

where $\gamma = 1/2 + \dfrac{\sqrt(3)}{3} \cos( \dfrac{\pi}{18}) \approx 1.06857902130162881$

The stability function for this method is:

!equation
R(z) = \dfrac{-0.76921266689461 z^3 - 0.719846311954034 z^2 + 2.20573706179581 z - 1.0}{
       1.22016884572716 z^3 - 3.42558337748497 z^2 + 3.20573706551682 z - 1.0}


The method is *not* L-stable, it is only A-stable:

!equation
\lim_{z->\infty} R(z) = -0.630414937726258

## Notes:
1.) Method is originally due to [!cite](crouzeix1975)

2.) Since $\gamma$ is slightly larger than 1, the first stage involves
    evaluation of the non-time residuals for t > t_n+dt, while the
    third stage involves evaluation of the non-time residual for t
    < t_n, which may present an issue for the first timestep (if
    e.g. material properties or forcing functions are not defined
    for t<0. We could handle this by using an alternate (more
    expensive) method in the first timestep, or by using a
    lower-order method for the first timestep and then switching to
    this method for subsequent timesteps.


!syntax parameters /Executioner/TimeIntegrator/AStableDirk4

!syntax inputs /Executioner/TimeIntegrator/AStableDirk4

!syntax children /Executioner/TimeIntegrator/AStableDirk4
