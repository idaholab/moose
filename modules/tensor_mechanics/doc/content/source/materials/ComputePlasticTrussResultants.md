# Plastic Truss

!syntax description /Materials/ComputePlasticTrussResultants

## Description

The `ComputePlasticTrussResultants` model implements J2 plasticity for 1D truss elements. The axial strain increment of the element is obtained from
\begin{equation}
\Delta\varepsilon = {\varepsilon}_{total}^{t} - {\varepsilon}_{total}^{t_{old}} \, ,
\end{equation}
where ${\varepsilon}_{total}^{t}$ is the total strain at current time step and ${\varepsilon}_{total}^{t_{old}}$ is the total strain at the previous time step. Here, the nonlinear behavior of the truss is implemented using a J2 plasticity model that can use either simple linear hardening or a user-defined function to define the hardening behavior.
The trial stress is estimated as
\begin{equation}
\sigma^{tr} = \sigma_{old} + E \Delta \varepsilon \, .
\end{equation}

The yield condition is determined as
\begin{equation}
f = |\sigma^{tr}| - E \Delta \varepsilon^p - r -\sigma_y = 0 \, ,
\end{equation}
where $E$ is the Young's modulus, $r$ is the hardening function and $\sigma_y$ is the yield stress. When the trial stress is outside of the yield envelop the stresses are brought down using the iterative Newton method.
In the case of linear hardening, the hardening function is defined as $r=h |\varepsilon^p|$ with $h$ being the hardening constant. In this case, the hardening variable at the beginning of the iterative process is obtained as
\begin{equation}
r^{(k)} = r_{old} + h  (\Delta \varepsilon^p)^{(k)} \, .
\end{equation}
The plastic strain increment is computed as
\begin{equation}
d\Delta \varepsilon^p = \frac{f}{\frac{df}{d\Delta \varepsilon^p}} = \frac{(|\tilde \sigma^{tr}| - E (\Delta \varepsilon^p)^{(k)} - r^k -\sigma_y)}{E + h} = 0 \, .
\end{equation}
The plastic strain for the next iteration is updated
\begin{equation}
(\Delta \varepsilon^p)^{(k+1)} = (\Delta \varepsilon^p)^{(k)} + d\Delta \varepsilon^p \, .
\end{equation}
The iterative process continues until the updated stress lies on the yield curve.
Then the elastic strain is updated as
\begin{equation}
\Delta \varepsilon^e = \Delta \varepsilon - \Delta \varepsilon^p \, .
\end{equation}
The updated axial stress is calculated
\begin{equation}
\sigma_a = \sigma_{old} + E \Delta \varepsilon^e \, ,
\end{equation}
Axial force on the element is calculated as
\begin{equation}
f_a = f_{a|old} + E A \Delta \varepsilon^e \, ,
\end{equation}

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/truss/truss_plastic_new.i block=Materials/truss

!syntax parameters /Materials/ComputePlasticTrussResultants

!syntax inputs /Materials/ComputePlasticTrussResultants

!syntax children /Materials/ComputePlasticTrussResultants
