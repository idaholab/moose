# KKSPhaseChemicalPotential

!syntax description /Kernels/KKSPhaseChemicalPotential

Enforces the point wise
equality of the phase chemical potentials

$$
\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}.
$$

The non-linear variable of this Kernel is $c_a$.

### Residual

$$
R=\frac{dF_a}{dc_a} - \frac{dF_b}{dc_b}
$$

### Jacobian

For the Jacobian we need to calculate

$$
J=\frac \partial{\partial u_j}\left( \frac{dF_a}{dc_a} - \frac{dF_b}{dc_b} \right).
$$

#### On-Diagonal

$$
J = \phi_j \left( \frac{\partial^2 F_a}{\partial c_a^2} - \frac{\partial^2 F_b}{\partial c_a \partial c_b} \right)
$$

#### Off-Diagonal

With $q$ the union of the argument vectors of $F_a$ and $F_b$ (represented in the code by `_coupled_moose_vars[]`) we get

$$
\sum_i \left( \frac{\partial^2 F_a}{\partial c_a \partial q_i}\frac{\partial q_i}{\partial u_j} - \frac{\partial^2 F_b}{\partial c_b \partial q_i}\frac{\partial q_i}{\partial u_j} \right).
$$

Again the $\frac{\partial q_i}{\partial u_j}$ is non-zero only if $u\equiv q_i$, which is the case if $q_i$ is the argument selected through `jvar`.

$$
J = \frac{\partial^2 F_a}{\partial c_a \partial q_\text{jvar}}\phi_j - \frac{\partial^2 F_b}{\partial c_b \partial q_\text{jvar}}\phi_j.
$$

Note that in the code `jvar` is not an index into `_coupled_moose_vars[]` but has to be resolved through the `_jvar_map`.

!syntax parameters /Kernels/KKSPhaseChemicalPotential

!syntax inputs /Kernels/KKSPhaseChemicalPotential

!syntax children /Kernels/KKSPhaseChemicalPotential
