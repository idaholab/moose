# LevelSetOlssonBubble

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

For example, the following code creates a "bubble" in the lower left corner in a domain ranging from
0 to 1 in the x- and y-direction, as shown in the figure.

!listing modules/level_set/test/tests/functions/olsson_bubble/olsson_bubble.i block=Functions

!syntax parameters /Functions/LevelSetOlssonBubble

!syntax inputs /Functions/LevelSetOlssonBubble

!syntax children /Functions/LevelSetOlssonBubble

!bibtex bibliography
