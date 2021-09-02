# Bilinear mixed mode traction separation law

This class implements the bilinear mixed mode traction separation law described in [!cite](Camanho2002).

## Softening onset prediction

The initiation of the softening process is predicted using the quadratic failure criterion given below,

\begin{equation}
\left(\frac{\langle\tau_3\rangle}{N}\right)^2 + \left(\frac{\tau_2}{S} \right)^2 + \left(\frac{\tau_1}{T} \right)^2 = 1
\end{equation}

The total mixed-mode relative displacement $\delta_m$ is defined as
\begin{equation}
\delta_m = \sqrt{\delta_1^2+\delta_2^2+\langle\delta_3\rangle^2} = \sqrt{\delta_{shear}^2+\langle\delta_3\rangle^2}
\end{equation}
where $\delta_{shear}$ represents the norm of the vector defining the tangential relative displacements of the element.

Using the same penalty stiffness in Modes I, II and III, the tractions before softening onset are:
\begin{equation}
\tau_i = K\delta_i, ~~~\text{i=1,2,3}
\end{equation}

Assuming $S=T$, the single mode relative displacements at softening onset are:
\begin{equation}
\delta_3^0=\frac{N}{K}
\end{equation}

\begin{equation}
\delta_1^0=\delta_2^0=\delta_{shear}^0=\frac{S}{K}
\end{equation}

For an opening displacement $\delta_3$ greater than zero, the mode mixity ratio $\beta$ is defined as:
\begin{equation}
\beta = \frac{\delta_{shear}}{\delta_3}
\end{equation}

The mixed-mode relative displacement corresponding to the onset of softening $\delta_m^0$ is given as
\begin{equation}
\delta_m^0=
\begin{cases}
    \delta_3^0\delta_1^0\sqrt{\frac{1+\beta^2}{(\delta_1^0)^2+(\beta\delta_3^0)^2}} , & \delta_3> 0\\
    \delta_{shear}^0,              & \delta_3\leq 0
\end{cases}
\end{equation}

## Delamination propagation prediction

### Power law criterion

The power law criterion is given as
\begin{equation}
\left(\frac{G_{I}}{G_{IC}}\right)^{\alpha} + \left(\frac{G_{II}}{G_{IIC}}\right)^{\alpha} = 1
\end{equation}

The mixed-mode displacements corresponding to total decohesion is given as:
\begin{equation}
\delta_m^f=
\begin{cases}
    \ \frac{2(1+\beta^2)}{K\delta_m^0}\left[\left(\frac{1}{G_{IC}}\right)^{\alpha} + \left(\frac{\beta^2}{G_{IIC}}\right)^{\alpha}\right]^{-1/\alpha} , & \delta_3> 0\\
    \ \sqrt{(\delta_1^f)^2+(\delta_2^f)^2},              & \delta_3\leq 0
\end{cases}
\end{equation}

### B-K criterion

The mixed-mode criterion proposed by Benzeggagh and Kenane is given as (B-K criterion):
\begin{equation}
G_{IC}+(G_{IIC}-G_{IC})\left(\frac{G_{shear}}{G_T}\right)^{\eta} = G_C~\text{with}~G_T=G_I+G_{shear}
\end{equation}

The mixed-mode displacements corresponding to total decohesion is given as:
\begin{equation}
\delta_m^f=
\begin{cases}
    \ \frac{2}{K\delta_m^0}\left[G_{IC}+(G_{IIC}-G_{IC}\left(\frac{\beta^2}{1+\beta^2}\right)^\eta)\right] , & \delta_3> 0\\
    \ \sqrt{(\delta_1^f)^2+(\delta_2^f)^2},              & \delta_3\leq 0
\end{cases}
\end{equation}

## Constitutive equation for mixed-mode loading

The constitutive equation for mixed-mode loading is given as
\begin{equation}
\tau_s = D_{sr}\delta_r
\end{equation}

\begin{equation}
D_{sr} =
\begin{cases}
\bar{\delta}_{sr}K, \text{if}~\delta_m^{max} \leq \delta_m^0 \\
\bar{\delta}_{sr}\left[(1-d)K+Kd\bar{\delta}_{s3}\frac{\langle-\delta_3\rangle}{-\delta_3}\right], \text{if}~\delta_m^0 < \delta_m^{max} \leq \delta_m^f \\
\bar{\delta}_{s3}\bar{\delta}_{3r}\frac{\langle-\delta_3\rangle}{-\delta_3}K, \text{if}~\delta_m^{max} \geq \delta_m^f
\end{cases}
\label{traction}
\end{equation}

\begin{equation}
d= \frac{\delta_m^f(\delta_m^{max}-\delta_m^0)}{\delta_m^{max}(\delta_m^f-\delta_m^0)}, d\in[0,1]
\label{damage}
\end{equation}

## Solver options

### Viscous regularization

Cohesive zone models exhibiting softening behavior and stiffness degradation often lead to convergence difficulties in an implicit solver. The traction-separation laws can be regularized using viscosity. The viscous damage variable $d_v$ is defined by
\begin{equation}
\dot{d_v}=\frac{1}{\mu}(d-d_v)
\end{equation}
where $\mu$ is the viscosity parameter representing the relaxation time of the viscous system. An analytical expression of $d_v$ can be obtained by using the backward Euler method. With viscous regularization, the $d$ will be replaced by $d_v$ in [traction] to compute traction.   

### Lag separation state

It is typically useful to improve convergence by lagging the separation state. When `lag_seperation_state = true`, the $\delta_3$, $\delta_m^{max}$, $\delta_m^0$ and $\delta_m^f$ will be replaced by their old values from previous time step.

### Use Regularized Heavyside Function

The step (heavyside) function $\frac{\langle-\delta_3\rangle}{-\delta_3}$ in [traction] usually makes convergence bad. In the code, we replaced it with the regularized heavside function which provides a C0 continuity. The regularization parameter can be set by `alpha` parameter.

## Examples

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/bilinear_mixed.i block=Materials/czm
!syntax parameters /Materials/BiLinearMixedModeTraction
!syntax inputs /Materials/BiLinearMixedModeTraction
!syntax children /Materials/BiLinearMixedModeTraction
