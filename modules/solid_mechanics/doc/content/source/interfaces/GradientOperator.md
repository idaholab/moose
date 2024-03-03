# Gradient operators

## 3D Cartesian coordinates id=3D_cartesian

The reference coordinates of a Cartesian coordinate system can be expressed as:
\begin{equation}
  \boldsymbol{X} = X \textbf{e}_X + Y \textbf{e}_Y + Z \textbf{e}_Z.
\end{equation}
The current coordinates can be expressed as:
\begin{equation}
  \boldsymbol{x} = x \textbf{e}_x + y \textbf{e}_y + z \textbf{e}_z,
\end{equation}
and the underlying motion is
\begin{equation}
  \begin{aligned}
    x(X,Y,Z) &= X + u_x(X,Y,Z), \\
    y(X,Y,Z) &= Y + u_y(X,Y,Z), \\
    z(X,Y,Z) &= Z + u_z(X,Y,Z).
  \end{aligned}
\end{equation}

The gradient operator (with respect to the reference coordinates) is given as
\begin{equation}
  (\cdot)_{,\boldsymbol{X}} = (\cdot)_{,X}\textbf{e}_X + (\cdot)_{,Y}\textbf{e}_Y + (\cdot)_{,Z}\textbf{e}_Z,
\end{equation}
and the deformation gradient is given as
\begin{equation}
  \begin{aligned}
    \boldsymbol{F} = &\ (1+u_{x,X}) \textbf{e}_x\textbf{e}_X + u_{x,Y} \textbf{e}_x\textbf{e}_Y + u_{x,Z} \textbf{e}_x\textbf{e}_Z \\
    &+ u_{y,X} \textbf{e}_y\textbf{e}_X + (1+u_{y,Y}) \textbf{e}_y\textbf{e}_Y + u_{y,Z} \textbf{e}_y\textbf{e}_Z \\
    &+ u_{z,X} \textbf{e}_z\textbf{e}_X + u_{z,Y} \textbf{e}_z\textbf{e}_Y + (1+u_{z,Z}) \textbf{e}_z\textbf{e}_Z.
  \end{aligned}
\end{equation}

Components of the gradient operator are
\begin{equation}
  \begin{aligned}
    \nabla^x \phi_x &= \phi_{x,X}\textbf{e}_x\textbf{e}_X + \phi_{x,Y}\textbf{e}_x\textbf{e}_Y + \phi_{x,Z}\textbf{e}_x\textbf{e}_Z, \\
    \nabla^y \phi_y &= \phi_{y,X}\textbf{e}_y\textbf{e}_X + \phi_{y,Y}\textbf{e}_y\textbf{e}_Y + \phi_{y,Z}\textbf{e}_y\textbf{e}_Z, \\
    \nabla^z \phi_z &= \phi_{z,X}\textbf{e}_z\textbf{e}_X + \phi_{z,Y}\textbf{e}_z\textbf{e}_Y + \phi_{z,Z}\textbf{e}_z\textbf{e}_Z.
  \end{aligned}
\end{equation}

## 2D axisymmetric cylindrical coordinates id=2D_axisymmetric_cylindrical

The reference coordinates of an axisymmetric cylindrical coordinate system can be expressed in terms of the radial coordinate $R$, the axial coordinate $Z$, and unit vectors $\textbf{e}_R$ and $\textbf{e}_Z$:
\begin{equation}
  \boldsymbol{X} = R \textbf{e}_R + Z \textbf{e}_Z.
\end{equation}
The current coordinates can be expressed in terms of coordinates and unit vectors in the current (displaced) configuration:
\begin{equation}
  \boldsymbol{x} = r \textbf{e}_r + z \textbf{e}_z,
\end{equation}
and the underlying motion is
\begin{equation}
  \begin{aligned}
    r(R,Z) &= R + u_r(R,Z), \\
    z(R,Z) &= Z + u_z(R,Z), \\
    \theta(\Theta) &= \Theta + u_\theta(\Theta).
  \end{aligned}
\end{equation}
Notice that the motion is assumed to be torsionless and $\nabla_{\Theta} \boldsymbol{x} = 0$.

In axisymmetric cylindrical coordinates, the gradient operator (with respect to the reference coordinates) is given as
\begin{equation}
  (\cdot)_{,\boldsymbol{X}} = (\cdot)_{,R}\textbf{e}_R + (\cdot)_{,Z}\textbf{e}_Z + \frac{1}{R}(\cdot)_{,\Theta}\textbf{e}_\Theta,
\end{equation}
and the deformation gradient is given as
\begin{equation}
  \begin{aligned}
    \boldsymbol{F} &= (1+u_{r,R}) \textbf{e}_r\textbf{e}_R + u_{r,Z} \textbf{e}_r\textbf{e}_Z + \left(1+\frac{u_r}{R}\right)\textbf{e}_\theta\textbf{e}_\Theta + u_{z,R} \textbf{e}_z\textbf{e}_R + (1+u_{z,Z}) \textbf{e}_z\textbf{e}_Z
  \end{aligned}
\end{equation}

Components of the gradient operator are
\begin{equation}
  \begin{aligned}
    \nabla^r \phi_r &= \phi_{r,R}\textbf{e}_r\textbf{e}_R + \phi_{r,Z}\textbf{e}_r\textbf{e}_Z + \frac{\phi_{r}}{R}\textbf{e}_\theta\textbf{e}_\Theta, \\
    \nabla^z \phi_z &= \phi_{z,R}\textbf{e}_z\textbf{e}_R + \phi_{z,Z}\textbf{e}_z\textbf{e}_Z.
  \end{aligned}
\end{equation}

## 1D centrosymmetric spherical coordinates id=1D_centrosymmetric_spherical

The reference coordinates of a centrosymmetric spherical coordinate system can be expressed in terms of the radial coordinate $R$ and the unit vector $\textbf{e}_R$:
\begin{equation}
  \boldsymbol{X} = R \textbf{e}_R.
\end{equation}
The current coordinates can be expressed in terms of the radial coordinate and the unit vector in the current (displaced) configuration:
\begin{equation}
  \boldsymbol{x} = r \textbf{e}_r,
\end{equation}
and the underlying motion is
\begin{equation}
  \begin{aligned}
    r(R,Z) &= R + u_r(R,Z), \\
    \theta(\Theta) &= \Theta + u_\theta(\Theta), \\
    \phi(\Phi) &= \Phi + u_\phi(\Phi). \\
  \end{aligned}
\end{equation}
Notice that the motion is torsionless.

In centrosymmetric spherical coordinates, the gradient operator (with respect to the reference coordinates) is given as
\begin{equation}
  (\cdot)_{,\boldsymbol{X}} = (\cdot)_{,R}\textbf{e}_R + \frac{1}{R}(\cdot)_{,\Theta}\textbf{e}_\Theta + \frac{1}{R}(\cdot)_{,\Phi}\textbf{e}_\Phi,
\end{equation}
and the deformation gradient is given as
\begin{equation}
  \begin{aligned}
    \boldsymbol{F} &= (1+u_{r,R}) \textbf{e}_r\textbf{e}_R + \left(1+\frac{u_r}{R}\right)\textbf{e}_\theta\textbf{e}_\Theta + \left(1+\frac{u_r}{R}\right)\textbf{e}_\phi\textbf{e}_\Phi.
  \end{aligned}
\end{equation}

Components of the gradient operator are
\begin{equation}
  \nabla^r \phi_r = \phi_{r,R}\textbf{e}_r\textbf{e}_R + \frac{\phi_{r}}{R}\textbf{e}_\theta\textbf{e}_\Theta + \frac{\phi_{r}}{R}\textbf{e}_\phi\textbf{e}_\Phi.
\end{equation}
