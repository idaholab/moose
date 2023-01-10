# Hill Plasticity Stress Update

!syntax description /Materials/HillPlasticityStressUpdate

## Description

This class computes Hill plasticity via a generalized radial return mapping algorithm [!cite](versino2018generalized). It
requires that the elastic behavior of the material is isotropic, whereas any departure from the yield function is anisotropic.
The Hill yield function can be defined as:
\begin{equation}
f := \frac{1}{2} \boldsymbol{s}^{\boldsymbol{T}} \boldsymbol{A} \boldsymbol{s} - {s_y}^2 \left(\alpha, \dot{\alpha}, T \right)
\label{eq:yield_condition}
\end{equation}
where $\boldsymbol{s}$ is the deviatoric stress tensor in Voigt form, $\boldsymbol{A}$ is the anisotropy (Hill) tensor, $T$ is the temperature and $\alpha$ is an internal parameter that can be used, for example, to prescribe strain hardening through a plasticity modulus. Hill's tensor is defined as a six by six matrix using the unitless constants $F$, $G$, $H$, $L$, $M$ and $N$ as following:

\begin{equation}
\boldsymbol{A} :=
  \begin{bmatrix}
  G+H & -H & -G & 0 & 0 & 0\\
  -H & F+H & -F & 0 & 0 & 0\\
  -G & -F & F+G & 0 & 0 & 0\\
  0 & 0 & 0 & 2N & 0 & 0\\
  0 & 0 & 0 & 0 & 2L & 0\\
  0 & 0 & 0 & 0 & 0 & 2M\\
  \end{bmatrix}
  \label{eq:Hill_tensor_definition}
\end{equation}

The model currently uses power law hardening:

\begin{equation}
\sigma_y = \sigma_{yo} + K \alpha^n
\label{eq:power_law_hardening}
\end{equation}

where $K$ is the hardening constant and $n$ is the hardening exponent. The hardening exponent has a default value of 1.0 and this value can be modified by the user by setting the parameter [!param](/Materials/HillPlasticityStressUpdate/hardening_exponent). The [!param](/Materials/HillPlasticityStressUpdate/yield_stress) ($\sigma_{yo}$) and [!param](/Materials/HillPlasticityStressUpdate/hardening_constant) ($K$) are the required parameters to be supplied by the user.

## Verification

With $f$ being the yield condition, $\boldsymbol{s}$ the deviatoric stress and $\gamma$ the plastic multiplier, the plastic strain rate is given as:

\begin{equation}
\dot{\boldsymbol{\epsilon}}^{\boldsymbol{p}} = \dot{\gamma}\frac{\partial f}{\partial \boldsymbol{s}}
\label{eq:normality_condition}
\end{equation}

[eq:normality_condition] is a statement of normality condition, i.e., the associative flow rule. Using [eq:yield_condition], the associative flow rule in [eq:normality_condition] can be written as:

\begin{equation}
\dot{\boldsymbol{\epsilon}}^{\boldsymbol{p}} = \dot{\gamma} \boldsymbol{A} \boldsymbol{s}
\label{eq:normality_condition2}
\end{equation}

which can be written in incremental form as:

\begin{equation}
\Delta\boldsymbol{\epsilon}^{\boldsymbol{p}} = \Delta\gamma \boldsymbol{A} \boldsymbol{s}
\label{eq:normality_condition2_incremental}
\end{equation}

If $\alpha$ is the internal state variable associated with isotropic hardening, and $\beta$ is the energy conjugate to the $\dot{\alpha}$, the $\dot{\alpha}$ is written as:

\begin{equation}
\dot{\alpha} = - \dot{\gamma}\frac{\partial f}{\partial \beta}
\label{eq:alpha_dot}
\end{equation}

The thermodynamic variable $\beta$ is $s_y$ here (see [!cite](versino2018generalized) for details). Equation [eq:alpha_dot] can be written in integrated incremental form as:

\begin{equation}
\Delta\alpha = 2 s_y \Delta\gamma
\label{eq:delta_alpha}
\end{equation}

Using [eq:delta_alpha] in [eq:normality_condition2_incremental], taking the partial derivative of $\Delta\boldsymbol{\epsilon}^{\boldsymbol{p}}$ with respect to $\boldsymbol{s}$, and applying chain rule, we obtain:

\begin{equation}
\frac{\partial \Delta\boldsymbol{\epsilon}^{\boldsymbol{p}}}{\partial \boldsymbol{s}} = \frac{\partial \Delta\boldsymbol{\epsilon}^{\boldsymbol{p}}}{\partial \Delta\alpha} \frac{\partial \Delta\alpha}{\partial \boldsymbol{s}} = \frac{\boldsymbol{A} \boldsymbol{s}}{2s_y} \frac{\partial \Delta\alpha}{\partial \boldsymbol{s}}
\label{eq:inverse_slope}
\end{equation}

\begin{equation}
\Rightarrow \frac{\partial \boldsymbol{s}}{\partial \Delta\boldsymbol{\epsilon}^{\boldsymbol{p}}} = \frac{2s_y}{\boldsymbol{A} \boldsymbol{s}} \frac{\partial \boldsymbol{s}}{\partial \Delta\alpha}
\label{eq:slopeA}
\end{equation}

For the case where flow rule involves linear hardening with hardening constant $K$:

\begin{equation}
\frac{\partial \boldsymbol{s}}{\partial \Delta\alpha} = K
\label{eq:slopeB}
\end{equation}

Substituting [eq:slopeB] in [eq:slopeA] we obtain:

\begin{equation}
\Rightarrow \frac{\partial \boldsymbol{s}}{\partial \Delta\boldsymbol{\epsilon}^{\boldsymbol{p}}} = \frac{2s_y}{\boldsymbol{A} \boldsymbol{s}} K
\label{eq:slopeC}
\end{equation}

The left hand side of [eq:slopeC] gives the slope of the stress vs plastic strain curve. This equation can be written for the direct components of stress ($s_{ii}$) as:

\begin{equation}
\Rightarrow \frac{\partial s_{xx}}{\partial \Delta\epsilon^p_{xx}} = \frac{2s_y}{(G+H) s_{xx}} K
\label{eq:slope_componentx}
\end{equation}

For a uniaxial tensile test with monotonically increasing load, the stress always lies on the yield surface once the plastic deformation starts (for loading in x-direction $s_{xx}$ = $s_y$) which simplifies [eq:slope_componentx] to:

\begin{equation}
\Rightarrow \frac{\partial s_{xx}}{\partial \Delta\epsilon^p_{xx}} = \frac{2K}{(G+H)}
\label{eq:slope_componentx2}
\end{equation}

Similarly, for uniaxial loading in y-direction we obtain:

\begin{equation}
\Rightarrow \frac{\partial s_{yy}}{\partial \Delta\epsilon^p_{yy}} = \frac{2K}{(F+H)}
\label{eq:slope_componenty}
\end{equation}

These simplified equations ([eq:slope_componentx2] & [eq:slope_componenty]) can be used in the verification tests for this material model.

!alert warning
The combination of elastic isotropy and plastic anisotropy should be solved by the more efficient [HillPlasticityStressUpdate](/HillPlasticityStressUpdate.md) class.

The effective plastic strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[GeneralizedRadialReturnStressUpdate](/GeneralizedRadialReturnStressUpdate.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/anisotropic_plasticity/ad_aniso_plasticity_y.i block=Materials/trial_plasticity

!syntax parameters /Materials/HillPlasticityStressUpdate

!syntax inputs /Materials/HillPlasticityStressUpdate

!syntax children /Materials/HillPlasticityStressUpdate

!bibtex bibliography
