# KKSPhaseConcentration
!syntax description /Kernels/KKSPhaseConcentration

Enforces the split of the
concentration into the phase concentrations, weighted by the switching function.
The non-linear variable of this Kernel is $c_b$.

$$
c = [1-h(\eta)]c_a+h(\eta)c_b
$$

### Residual

$$
R=[1-h(\eta)]c_a + h(\eta)c_b - c
$$

### Jacobian

#### On-Diagonal

Since the non-linear variable is $c_b$,

$$
J= \phi_j \frac{\partial R}{\partial c_b} = \phi_j h(\eta)
$$

#### Off-Diagonal

For $c_a$

$$
J= \phi_j \frac{\partial R}{\partial c_a} = \phi_j [1-h(\eta)]
$$

For $c$

$$
J= \phi_j \frac{\partial R}{\partial c} = -\phi_j
$$

For $\eta$

$$
J= \phi_j \frac{\partial R}{\partial \eta} = \phi_j \frac{dh}{d\eta}(c_b-c_a)
$$

!syntax parameters /Kernels/KKSPhaseConcentration

!syntax inputs /Kernels/KKSPhaseConcentration

!syntax children /Kernels/KKSPhaseConcentration
