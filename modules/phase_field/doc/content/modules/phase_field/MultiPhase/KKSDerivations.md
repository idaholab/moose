# KKS Model Definitions

\begin{equation}
F = [1-h(\eta)]F_a + h(\eta)F_b+wg(\eta)
\end{equation}

\begin{equation}
c =  [1-h(\eta)]c_a + h(\eta)c_b
\end{equation}

\begin{equation}
\frac{dF_a}{dc_a} = \frac{dF_b}{dc_b}
\end{equation}

# Materials

The KKS system ([!cite](kim_phase-field_1999)) uses multiple MOOSE materials to provide
values for free energy  functions, the switching function $h(\eta)$, and the double
well function $g(\eta)$. Providing these as materials allows the functions to be
bundled in a single place, while being used by multiple kernels. Furthermore the
automatic differentiation feature used in the parsed function materials prreplovides
the necessary derivatives at no cost to the developer. The derivatives are stored
in material properties and follow a naming scheme that is defined in `KKSBaseMaterial.C`.

# Cahn-Hilliard Kernels

## KKSSplitCHCRes

[`KKSSplitCHCRes`](/KKSSplitCHCRes.md) is the split version. In this kernel, we calculate the chemical potential
$\mu$ from $\frac{\partial F}{\partial c}$. The non-linear variable for this Kernel
is the concentration $c$. To calculate $\frac{\partial c}{\partial t}$ and
$\nabla \cdot M(c) \nabla \mu$, we use the [`CoupledTimeDerivative`](/CoupledTimeDerivative.md) and
[`SplitCHWRes`](/SplitCHWRes.md) kernels, respectively, as described [here](phase_field/Phase_Field_Equations.md).

## Residual

In the residual routine we need to calculate the term $R= \frac{\partial F}{\partial c} - \mu$.
We exploit the KKS identity $\frac{\partial F}{\partial c}=\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}$
and arbitrarily use the a-phase instead.

\begin{equation}
R = \frac{\partial F_a}{\partial c_a} - \mu
\end{equation}

### Jacobian

#### On-Diagonal

Since there is no explicit dependence on the non-linear variable $c$ in the residual
equation, the diagonal components are zero.

#### Off-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv c_a$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial c_a}{\partial u_j}\frac{\partial}{\partial c_a}=\phi_j \frac{\partial}{\partial c_a}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \frac{\partial R}{\partial u_j} = \phi_j \frac{\partial}{\partial c_a} \left( \frac{\partial F}{\partial c_a} - \mu \right)\\
&=& \phi_j  \frac{\partial^2F}{\partial c_a^2} \\
\end{aligned}
\end{equation}

For $\mu$

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial \mu} \left( \frac{\partial F}{\partial c_a} - \mu \right)\\
&=& -\phi_j \\
\end{aligned}
\end{equation}

## KKSCHBulk

[`KKSCHBulk`](/KKSCHBulk.md) is the non-split version, which is **not fully implemented**.
The non-linear variable for this Kernel is the concentration $c$.

### Residual

In the residual routine we need to calculate the term $R=\nabla \frac{dF}{dc}$.
We exploit the KKS identity $\frac{dF}{dc}=\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}$
and arbitrarily use the a-phase instead.
The gradient can be calculated through the chain rule (note that $F_a(c_a, p_1,p_2,\dots,p_n)$
is potentially a function of many variables).

\begin{equation}
R = \nabla \frac{dF_a}{dc_a} = \frac{d^2F_a}{dc_a^2}\nabla c_a + \sum_i \frac{d^2F_a}{dc_adp_i}\nabla p_i
\end{equation}

With $a = \{c_a, p_1,p_2,\dots,p_n\}$ being the vector of all arguments to $F_a$ this simplifies to

\begin{equation}
R=\sum_i \frac{d^2F_a}{dc_ada_i}\nabla a_i  = \sum_i R_i \nabla a_i
\end{equation}

using $R_i$ as a shorthand for the term $\frac{d^2F_a}{dc_ada_i}$ (and represented
in the code as the array `_second_derivatives[i]`). We do have access to the
gradients of $a_i$ through MOOSE (stored in `_grad\_args[i]`).

### Jacobian

The calculation of the Jacobian involves the derivative of the Residual term $R$
w.r.t. the individual coefficients $u_j$ of all parameters of $F_a$. Here $u$ can
stand for any variable $a_i$.

\begin{equation}
\frac{dR}{du_j} = \sum_i \left[ \frac d{du_j} R_i\nabla a_i \right] = \sum_i \left[  R_i\frac{d\nabla a_i}{du_j} + \nabla a_i \sum_k \frac {dR_i}{da_k}\frac{da_k}{du_j} \right]
\end{equation}

In the code $u$ is given by `jvar` for the off diagonal case, and $c$
(not $c_a$ or $c_b$!) in the on diagonal case.

#### Off-diagonal

Let's focus on off diagonal first. Here $\frac{da_k}{du_j}$ is zero, if `jvar`
is not equal $k$. Allowing us to remove the sum over $k$ and replace it with the
single non-zero summand

\begin{equation}
\frac{dR}{du_j} = \sum_i \left[  R_i\frac{d\nabla a_i}{du_j} + \nabla a_i \frac {dR_i}{da_\text{jvar}}\frac{da_\text{jvar}}{du_j} \right]
\end{equation}

In the first term in the square brackets the derivative $\frac{d\nabla a_i}{du_j}$
is only non-zero if $i$ is `jvar`. We can therefore pull this term out of the
sum.

\begin{equation}
\frac{dR}{du_j} = R_\text{jvar}\frac{d\nabla a_\text{jvar}}{du_j} + \sum_i  \nabla a_i \frac {dR_i}{da_\text{jvar}}\frac{da_\text{jvar}}{du_j}
\end{equation}

With the rules for $\frac d{du_j}$ derivatives we get

\begin{equation}
R_\text{jvar} \nabla \phi_j + \sum_i \nabla a_i \frac {dR_i}{da_\text{jvar}} \phi_j
\end{equation}

where $j$ is `_j` in the code.

#### On-diagonal

For the on diagonal terms we look at the derivative w.r.t. the components of the
non-linear variable $c$ of this kernel. Note, that $F_a$ is only indirectly a
function of $c$. We assume the dependence is given through $c(c_a)$. The chain
rule will thus yield terms of this form

\begin{equation}
\frac{dc_a}{dc} = \frac{\frac{d^2F_b}{dc_b^2}}{[1-h(\eta)]\frac{d^2F_b}{dc_b^2}+h(\eta)\frac{d^2F_a}{dc_a^2}},
\end{equation}

which is given as equation (23) in KKS. Following the off-diagonal  derivation we get

v
\frac{d^2F_a}{dc_a^2}\frac{dc_a}{dc} \nabla \phi_j + \sum_i \nabla a_i \frac {dR_i}{dc_a} \frac{dc_a}{dc} \phi_j
\end{equation}

#### On-diagonal second approach

Let's get back to the original residual with $\frac{dF}{dc}$. Then

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{d}{dc} \nabla \frac{dF}{dc}\\
&=& \phi_j  \nabla \frac{d^2F}{dc^2} \quad,\quad \text{with (29) from KKS}\\
&=& \phi_j  \nabla \frac{\frac{d^2F_b}{dc_b^2}\frac{d^2F_a}{dc_a^2}}{  [1-h(\eta)]\frac{d^2F_b}{dc_b^2}+h(\eta)\frac{d^2F_a}{dc_a^2} }\\
\end{aligned}
\end{equation}

# Allen-Cahn  Kernels

For the bulk Allen-Cahn residual we need to calculate the term

\begin{equation}
\begin{aligned}
R=\frac{dF}{d\eta}&=&\frac{d}{d\eta}\left([1-h(\eta)]F_a + h(\eta)F_b+wg(\eta) \right)\\
&=& \frac{dF_a}{d\eta}+\frac{d}{d\eta}\left(h(\eta)F_b-h(\eta)F_a + wg(\eta)\right)\\
&=& \frac{dF_a}{d\eta}+F_b\frac{dh}{d\eta}-F_a\frac{dh}{d\eta}+h(\eta)\frac{dF_b}{d\eta}-h(\eta)\frac{dF_a}{d\eta} +w\frac{dg}{d\eta}\\
&=& \frac{dF_a}{d\eta}+\frac{dh}{d\eta}(F_b-F_a)+h(\eta)\left(\frac{dF_b}{d\eta}-\frac{dF_a}{d\eta}\right) +w\frac{dg}{d\eta}\\
&=& \frac{dh}{d\eta}(F_b-F_a) + \underbrace{[1-h(\eta)]\frac{dF_a}{d\eta} + h(\eta)\frac{dF_b}{d\eta}}_{\text{chain rule term}} +w\frac{dg}{d\eta}\\
&=& \frac{dh}{d\eta}(F_b-F_a) + [1-h(\eta)]\frac{dF_a}{dc_a}\frac{dc_a}{d\eta} + h(\eta)\frac{dF_b}{dc_b}\frac{dc_b}{d\eta} +w\frac{dg}{d\eta}\\
&=& \frac{dh}{d\eta}(F_b-F_a) + \frac{dF_a}{dc_a}\left([1-h(\eta)]\frac{dc_a}{d\eta} + h(\eta)\frac{dc_b}{d\eta}\right) +w\frac{dg}{d\eta}
\end{aligned}
\end{equation}

The _chain rule term_ results from the fact that $c_a$ and $c_b$ are dependent
on $\eta$ (see eqs. (25) and (26) in KKS). Setting
$\lambda = [1-h(\eta)]\frac{d^2F_b}{dc_b^2}+h(\eta)\frac{d^2F_a}{dc_a^2}$ we get

\begin{equation}
\begin{aligned}
\frac{dc_a}{d\eta} &=& \frac1\lambda \frac{dh}{d\eta}(c_a-c_b)\frac{d^2F_b}{dc_b^2}\\
\frac{dc_b}{d\eta} &=& \frac1\lambda \frac{dh}{d\eta}(c_a-c_b)\frac{d^2F_a}{dc_a^2}.
\end{aligned}
\end{equation}

Substituting this in we get

\begin{equation}
\begin{aligned}
R &=& \frac{dh}{d\eta}(F_b-F_a) + \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b)\frac1\lambda\underbrace{\left([1-h(\eta)] \frac{d^2F_b}{dc_b^2} + h(\eta)\frac{d^2F_a}{dc_a^2}\right)}_{=\lambda} +w\frac{dg}{d\eta}.
\end{aligned}
\end{equation}

This simplifies to

\begin{equation}
R=-\frac{dh}{d\eta} \left(F_a-F_b-\frac{dF_a}{dc_a}(c_a-c_b)\right) + w\frac{dg}{d\eta}.
\end{equation}

We split this residual into two kernels to allow for multiple phase concentrations
in a multi component system:

## `KKSACBulkF`

[`KKSACBulkF`](/KKSACBulkF.md) is the part without a direct composition dependence.

### Residual

\begin{equation}
R=-\frac{dh}{d\eta}(F_a-F_b)+w\frac{dg}{d\eta}.
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta}=\phi_j \frac{\partial}{\partial\eta}$
derivative.

\begin{equation}
\begin{aligned}
J &=& -\phi_j \frac{\partial}{\partial \eta}\left( \frac{dh}{d\eta}(F_a-F_b) \right) + w \phi_j \frac{\partial}{\partial \eta}\frac{dg}{d\eta} \\
&=&-\frac{d^2h}{d\eta^2}\phi_j(F_a-F_b) + w\frac{d^2g}{d\eta^2}\phi_j \\
\end{aligned}
\end{equation}

(The implicit dependence of $F_a(c_a)$ and $F_b(c_a)$ on $\eta$ through $c_a(c,\eta)$
and $c_b(c,\eta)$ does not contribute to the Jacobian, so
$\frac{\partial F_a}{\partial \eta} = \frac{\partial F_a}{\partial \eta} = 0)$.

#### Off-Diagonal

The off-diagonal components are calculated for any other variables that $F_a$
and $F_b$ depend on. For example, for $c_a$:

\begin{equation}
J = \frac{dh}{d\eta}\left( \frac{\partial F_a}{\partial c_a} - \frac{\partial F_b}{\partial c_a}\right)\phi_j
\end{equation}

$\frac{\partial F_b}{\partial c_a} = 0$ in the KKS formulation, so this term
would not need to be included if $c_a$ was the only variable $F_a$ depended on.
However, the code calculates derivatives with respect to all variables that
$F_a$ and $F_b$ depend on in a general way so that the Jacobian entries for other
dependencies are correctly computed using the same piece of code. For example,
both $F_a$ and $F_b$ could depend on temperature $T$, in which case

\begin{equation}
J = \frac{dh}{d\eta}\left( \frac{\partial F_a}{\partial T} - \frac{\partial F_b}{\partial T}\right)\phi_j
\end{equation}

which is computed using the same code. The off-diagonal Jacobian contribution is
also multiplied by the Allen-Cahn mobility $L$ at each point for consistency with
the other terms in the Allen-Cahn equation.

## `KKSACBulkC`

[`KKSACBulkC`](/KKSACBulkC.md) is the part with a direct composition dependence.
An instance of this kernel is needed for each compnent of the problem.

### Residual

\begin{equation}
\begin{aligned}
R&=&-\frac{dh}{d\eta}\left(-\frac{dF_a}{dc_a}(c_a-c_b)\right)\\
&=&\frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b)
\end{aligned}
\end{equation}

### Jacobian

#### On-diagonal

We are looking for the $\frac \partial{\partial u_j}$ derivative of $R$, where
$u\equiv\eta$. We need to apply the chain rule and will again only keep terms
with the $\frac{\partial \eta}{\partial u_j}\frac{\partial}{\partial \eta} = \phi_j \frac{\partial}{\partial \eta}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial \eta} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{d^2h}{d\eta^2}\phi_j\frac{dF_a}{dc_a}(c_a-c_b)\\
\end{aligned}
\end{equation}

#### Off-diagonal

Since $c_a$ and $c_b$ appear in the residual, their effect must be calculated
separately from any other variable dependence. For $c_a$, we are looking for the
$\frac \partial{\partial u_j}$ derivative of $R$, where $u\equiv c_a$. We need to
apply the chain rule and will again only keep terms with the
$\frac{\partial c_a}{\partial u_j}\frac{\partial}{\partial c_a} = \phi_j \frac{\partial}{\partial c_a}$
derivative.

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial c_a} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{dh}{d\eta} \phi_j \left( \frac{d^2 F_a}{dc_a^2}(c_a-c_b) + \frac{d F_a}{d c_a}\right)\\
\end{aligned}
\end{equation}

Similarly for $c_b$,
\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial c_b} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& -\frac{dh}{d\eta} \phi_j  \frac{d F_a}{d c_a}\\
\end{aligned}
\end{equation}

For any variable other than $c_a$ or $c_b$, for example temperature $T$:

\begin{equation}
\begin{aligned}
J &=& \phi_j \frac{\partial}{\partial T} \left( \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \right)\\
&=& \frac{dh}{d\eta} \phi_j  \frac{\partial}{\partial T}\left(\frac{d F_a}{d c_a}\right) ( c_a - c_b)\\
\end{aligned}
\end{equation}


The off-diagonal Jacobian contributions are again multiplied by the Allen-Cahn
mobility $L$ at each point for consistency with the other terms in the Allen-Cahn
equation.

# Constraint Kernels

## `KKSPhaseChemicalPotential`

[`KKSPhaseChemicalPotential`](/KKSPhaseChemicalPotential.md) enforces the point wise
equality of the phase chemical potentials

\begin{equation}
\frac{dF_a}{dc_a}=\frac{dF_b}{dc_b}.
\end{equation}

The non-linear variable of this Kernel is $c_a$.

### Residual

\begin{equation}
R=\frac{dF_a}{dc_a} - \frac{dF_b}{dc_b}
\end{equation}

### Jacobian

For the Jacobian we need to calculate

\begin{equation}
J=\frac \partial{\partial u_j}\left( \frac{dF_a}{dc_a} - \frac{dF_b}{dc_b} \right).
\end{equation}

#### On-Diagonal

\begin{equation}
J = \phi_j \left( \frac{\partial^2 F_a}{\partial c_a^2} - \frac{\partial^2 F_b}{\partial c_a \partial c_b} \right)
\end{equation}

#### Off-Diagonal

With $q$ the union of the argument vectors of $F_a$ and $F_b$ (represented in the code by `_coupled_moose_vars[]`) we get

\begin{equation}
\sum_i \left( \frac{\partial^2 F_a}{\partial c_a \partial q_i}\frac{\partial q_i}{\partial u_j} - \frac{\partial^2 F_b}{\partial c_b \partial q_i}\frac{\partial q_i}{\partial u_j} \right).
\end{equation}

Again the $\frac{\partial q_i}{\partial u_j}$ is non-zero only if $u\equiv q_i$, which is the case if $q_i$ is the argument selected through `jvar`.

\begin{equation}
J = \frac{\partial^2 F_a}{\partial c_a \partial q_\text{jvar}}\phi_j - \frac{\partial^2 F_b}{\partial c_b \partial q_\text{jvar}}\phi_j.
\end{equation}

Note that in the code `jvar` is not an index into `_coupled_moose_vars[]` but has to be resolved through the `_jvar_map`.

## `KKSPhaseConcentration`

[`KKSPhaseConcentration`](/KKSPhaseConcentration.md) enforces the split of the
concentration into the phase concentrations, weighted by the switching function.
The non-linear variable of this Kernel is $c_b$.

\begin{equation}
c = [1-h(\eta)]c_a+h(\eta)c_b
\end{equation}

### Residual

\begin{equation}
R=[1-h(\eta)]c_a + h(\eta)c_b - c
\end{equation}

### Jacobian

#### On-Diagonal

Since the non-linear variable is $c_b$,

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c_b} = \phi_j h(\eta)
\end{equation}

#### Off-Diagonal

For $c_a$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c_a} = \phi_j [1-h(\eta)]
\end{equation}

For $c$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c} = -\phi_j
\end{equation}

For $\eta$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial \eta} = \phi_j \frac{dh}{d\eta}(c_b-c_a)
\end{equation}
