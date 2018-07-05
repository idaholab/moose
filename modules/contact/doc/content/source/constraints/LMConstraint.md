# LMConstraint

The `LMConstraint` enforces the following Karush-Kuhn-Tucker conditions for
normal mechanical contact:

\begin{equation}\label{eq:kkt}
\begin{aligned}
    g_N &\geq 0\\
    p_N &\geq 0\\
    g_N p_N &= 0
\end{aligned}
\end{equation}

where $g_N$ is the normal gap distance from the slave node to its projection on
the master face and $p_N$ is the normal contact pressure exterted between the
two bodies. The KKT conditions are implemented using a Nonlinear Complimentarity
Function (NCP), specifically the Fischer-Burmeister function:

\begin{equation}
    \phi_{FB}(a, b) = a + b - \sqrt{a^2 + b^2}
\end{equation}

where $a$ corresponds to the LHS on the first line in [eq:kkt] and $b$
corresponds to the LHS on the second. $p_N$ corresponds to a Lagrange Multiplier
and is an additional variable in the non-linear system.

## Summary

!syntax description /Constraints/LMConstraint

!syntax parameters /Constraints/LMConstraint

!syntax inputs /Constraints/LMConstraint

!syntax children /Constraints/LMConstraint

!bibtex bibliography
