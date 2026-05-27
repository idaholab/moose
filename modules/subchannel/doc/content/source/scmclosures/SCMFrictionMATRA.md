# SCMFrictionMATRA

!syntax description /SCMClosures/SCMFrictionMATRA

## Overview

!! Intentional comment to provide extra spacing

This class is used to model the axial friction factor for a subchannel assembly with bare fuel pins in a quadrilateral lattice. It implements the MATRA correlation [!cite](KIT). For Reynolds number ranges below $Re = 5000$, where the MATRA correlation is not applicable, SCM applies a custom extension that keeps the friction factor continuous at the transition to the MATRA correlation:

\begin{equation}
Re_c = \left(\frac{64}{0.316}\right)^{4/3},
\quad
\eta = \frac{Re - Re_c}{5000 - Re_c},
\quad
w = 3 \eta^2 - 2 \eta^3
\end{equation}

\begin{equation}
f_w \rightarrow
\begin{cases}
64, & Re < 1\\
\frac{64}{Re}, &1 \leq Re < Re_c\\
(1 - w)\frac{64}{Re} + w 0.316 Re^{-0.25}, &Re_c \leq Re < 5000\\
0.316 Re^{-0.25}, &5000 \leq Re < 30000\\
0.184 Re^{-0.20}, &30000 \leq Re < 1e6
\end{cases}
\end{equation}

!syntax parameters /SCMClosures/SCMFrictionMATRA

!syntax inputs /SCMClosures/SCMFrictionMATRA

!syntax children /SCMClosures/SCMFrictionMATRA
