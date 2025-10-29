# SCMFrictionMATRA

!syntax description /SCMClosures/SCMFrictionMATRA

## Overview

!! Intentional comment to provide extra spacing

This class is used to model the axial friction factor for a subchannel assembly with bare fuel pins in a quadrilateral lattice. It implements the MATRA correlation [!cite](KIT). For Re number ranges: $0 leq Re < 5000$ where the MATRA correlation is not applicable, a custom model has been applied:

\begin{equation}
f_w \rightarrow
\begin{cases}
64, & Re < 1\\
\frac{64}{Re}, &1 \leq Re<5000\\
0.316 Re^{-0.25}, &5000 \leq Re < 30000\\
0.184 Re^{-0.20}, &30000 \leq Re
\end{cases}
\end{equation}

!syntax parameters /SCMClosures/SCMFrictionMATRA

!syntax inputs /SCMClosures/SCMFrictionMATRA

!syntax children /SCMClosures/SCMFrictionMATRA
