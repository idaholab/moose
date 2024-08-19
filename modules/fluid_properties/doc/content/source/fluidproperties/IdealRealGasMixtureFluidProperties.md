# IdealRealGasMixtureFluidProperties

This class computes fluid properties for gaseous mixtures with a condensable
(primary) component and one or more non-condensable (secondary) components.
This model (which corresponds to "Model A" in [!citep](hansel2018ncgs))
assumes that each gas in the mixture occupies the entire mixture
volume at a common temperature and each has a partial pressure and is considered
an ideal gas mixture approximation applied to real gases.

## Formulation

Consider a mixture of $N$ gases. Let the $i$ subscript denote the component index
and the lack of a subscript denote a mixture quantity. The mass fraction $\xi_i$
is defined as the ratio of the component mass $m_i$ to the mixture mass $m$:

!equation id=massfrac
\xi_i = \frac{m_i}{m} = \frac{\rho_i}{\rho} = \frac{v}{v_i} \,,

where the densities $\rho_i$ and $\rho$ are with respect to the mixture volume. The molar fraction
$\psi_i$ is defined as the ratio of the component number of moles $n_i$ to the mixture
number of moles $n$:

!equation
\psi_i = \frac{n_i}{n} \,.

The mixture molar mass $M$ can be computed using the component molar fractions
and molar masses $M_i$:

!equation
M = \sum\limits_i^N \psi_i M_i \,.

The mass fraction and molar fraction are related as

!equation
\xi_i = \frac{\psi_i M_i}{M} \,.

Note the following relation for all mass-specific quantities, such as
specific volume and specific internal energy:

!equation
v = \sum\limits_i^N \xi_i v_i(p_i, T) \,,

!equation
e = \sum\limits_i^N \xi_i e_i(p_i, T) \,,

where $y_i(p_i, T)$ is the component equation of state call for $y$, evaluated
at the partial pressure and common temperature.

Note that the mixture density should be computed as

!equation
\rho = \frac{1}{v}

instead of a summation of component densities, since this would be
inconsistent if any component is not an ideal gas.

Transport properties, such as dynamic viscosity and thermal conductivity are
computed as

!equation id=viscosity
\mu = \sum\limits_i^N \psi_i \mu_i(p_i, T) \,,

!equation id=conductivity
k = \sum\limits_i^N \psi_i k_i(p_i, T) \,.

The gases share a temperature $T$, and each has a partial pressure $p_i$,
which is an assumption known as Dalton's law:

!equation
p = \sum\limits_i^N p_i(T, v_i) \,,

where by [!eqref](massfrac),

!equation
v_i = \frac{v}{\xi_i} \,.

Note

!equation
p_i = \psi_i p \,.

## Sound Speed and Specific Heat Capacities

The mixture sound speed and heat capacities are computed directly from thermodynamic definitions
relative to mixture properties, rather than constituent properties:

!equation id=cp
c_p = \left(\frac{\partial h}{\partial T}\right)_p \,,

!equation id=cv
c_v = \left(\frac{\partial e}{\partial T}\right)_v \,,

!equation id=soundspeed
c = v\sqrt{-\left(\frac{\partial p}{\partial v}\right)_s} \,.

Since the independent parameters are $p$ and $T$, the following relations can be used:

!equation
\left(\frac{\partial e}{\partial T}\right)_v = \left(\frac{\partial e}{\partial T}\right)_p
  - \left(\frac{\partial e}{\partial p}\right)_T
  \frac{\left(\frac{\partial v}{\partial T}\right)_p}
  {\left(\frac{\partial v}{\partial p}\right)_T} \,,

!equation
\left(\frac{\partial p}{\partial v}\right)_s = \left[\left(\frac{\partial v}{\partial p}\right)_T
  - \left(\frac{\partial v}{\partial T}\right)_p
  \frac{\left(\frac{\partial s}{\partial p}\right)_T}
  {\left(\frac{\partial s}{\partial T}\right)_p}\right]^{-1} \,.

## Implementation

The available interfaces are summarized in the following table, where the rows
correspond to the computed quantity, and the columns correspond to the various
combinations of input arguments. Note that the notation
$x : y(x) = y$ denotes the nonlinear solve of the equation $y(x) = y$ for $x$:

| Quantity | $(v,e,\xi)$ | $(p,T,\xi)$ | $(T,v,\xi)$ | $(p,v,\xi)$ | $(p,\rho,\xi)$ |
| :- | - | - | - | - | - |
| $v$ |   | $v: p(T,v,\xi) = p$ |   |   |   |
| $\rho$ |   | $\frac{1}{v(p,T,\xi)}$ |   |   |   |
| $T$ | $T: e(T,v,\xi) = e$ |   |   | $T: p(T,v,\xi) = p$ |   |
| $e$ |   | $e(T,v(p,T,\xi),\xi)$ | $\sum\limits_i^N \xi_i e_i(T, v/\xi_i)$ |   | $e(T(p,\frac{1}{\rho},\xi), \frac{1}{\rho}, \xi)$ |
| $s$ |   |   | $\sum\limits_i^N \xi_i s_i(T, v/\xi_i)$ |   |   |
| $p$ | $p(T(v,e,\xi),v,\xi)$ |   | $\sum\limits_i^N p_i(T,v/\xi_i)$ |   |   |
| $c$ | [!eqref](soundspeed) | [!eqref](soundspeed) | [!eqref](soundspeed) |   |   |
| $c_p$ |   | [!eqref](cp) | [!eqref](cp) |   |   |
| $c_v$ |   | [!eqref](cv) | [!eqref](cv) |   |   |
| $\mu$ |   | $\sum\limits_i^N \psi_i \mu_i(v_i, e_i(T, v_i))$ | [!eqref](viscosity) |   |   |
| $k$ |   | $\sum\limits_i^N \psi_i k_i(v_i, e_i(T, v_i))$ | [!eqref](conductivity) |   |   |

!syntax parameters /FluidProperties/IdealRealGasMixtureFluidProperties

!syntax inputs /FluidProperties/IdealRealGasMixtureFluidProperties

!syntax children /FluidProperties/IdealRealGasMixtureFluidProperties
