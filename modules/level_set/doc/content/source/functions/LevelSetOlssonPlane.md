# LevelSetOlssonPlane

[!cite](olsson2005conservative) define a level set function ($\Phi$) that differs from the traditional
signed distance function. They define a level set function ranging from 0 to 1 with a defined
thickness ($\epsilon$), which is a commonly referred to as a smeared Heaviside function
($H_{sm}(\Phi)$):

\begin{equation}
H_{sm}(\Phi) =
\begin{cases}
0, & \Phi < -\epsilon, \\
\frac{1}{2} + \frac{\Phi}{2\epsilon} + \frac{1}{2\pi}\sin(\frac{\pi\Phi}{\epsilon}), & -\epsilon \le \Phi \le \epsilon, \\
1, & \Phi > \epsilon.
\end{cases}
\end{equation}

Typically, the interface of the level set function is defined by the 0.5 contour and the interface or
boundary layer is defined between 0 and 1.

`LevelSetOlssonPlane` creates a plane that is defined by a point and a normal vector to the plane. The value is greater than 0.5 if the point is on the same side of the plane as the normal vector and less than 0.5 if it is on the opposite side.

!listing modules/level_set/test/tests/functions/olsson_plane/olsson_plane.i block=Functions

!syntax parameters /Functions/LevelSetOlssonPlane

!syntax inputs /Functions/LevelSetOlssonPlane

!syntax children /Functions/LevelSetOlssonPlane

!bibtex bibliography
