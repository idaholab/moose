# LevelSetAdvection

The level set equation is typically defined as below. As shown in this equation, the
`LevelSetAdvection` kernel implements the advection portion of the equation.

\begin{equation}
\frac{\partial u}{\partial t} + \underbrace{\vec{v} \cdot \nabla u}_{\textrm{LevelSetAdvection}} = 0,
\end{equation}
where $u$ is the level set variable, $t$ is time, and $\vec{v}$ is a known velocity field that
advects the level set variable.

The weak form of this portion of the equation is defined as: find $u_h$ such that:

\begin{equation}
(\Psi_i, \vec{v}\cdot\nabla u_h) = 0.
\end{equation}

## Example Syntax

This kernel is utilized from within the [`Kernels`](syntax/Kernels/index.md) block in conjunction
with a time derivative kernel ([TimeDerivative](/TimeDerivative.md)).

!listing modules/level_set/test/tests/reinitialization/parent.i block=Kernels

!syntax parameters /Kernels/LevelSetAdvection

!syntax inputs /Kernels/LevelSetAdvection

!syntax children /Kernels/LevelSetAdvection
