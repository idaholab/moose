# Crystal Plasticity State Var Rate Component Voce

!syntax description /UserObjects/CrystalPlasticityStateVarRateComponentVoce

## Description

This UserObject is meant to be used within the user object base crystal
plasticity framework as a Voce hardening rule.

\begin{equation}
\label{eq:self_hardening_rate}
h^\alpha = h_0^\alpha\left(1-\frac{\tau^{\alpha}_{CRSS}-\tau_0^\alpha}{\tau_{Sat}^\alpha-\tau_0^\alpha} \right)^{m^\alpha}
\end{equation}

with

\begin{equation}
\label{eq:slip_system_hardening_rate}
\dot\tau^\alpha_{CRSS} = \sum_{\beta=1}^N \lvert \dot\gamma^\beta \rvert h^\beta q^{\alpha \beta}
\end{equation}

where $\tau^\alpha_{CRSS}$ is the current critical resolved shear stress,
$\tau^{\alpha}_{0}$ is initial critical resolved shear stress,
$\tau_{Sat}^\alpha$ is the saturation resolved shear stress, $q^{\alpha
\beta}$ is the matrix containing self and latent hardening coefficients;
$h_0^\alpha$ and $m^\alpha$ are hardening parameters.

Like for the others crystal plasticity routines values of all the parameters can
be specified by groups of slip systems.

This UseroOject assumes that slip systems are provided in a well defined order
and are grouped by planes (see
`modules/tensor_mechanics/test/tests/cp_user_object/input_slip_sys_bcc48.txt` for
BCC crystals and
`modules/tensor_mechanics/test/tests/cp_user_object/input_slip_sys.txt` for FCC)

Seven variables need to be specified and one value is required for each group:

- `groups` in which groups of slip systems are listed
- `h0_group_values` $h_0$ value for each slip system group
- `tau0_group_values` $\tau_0$ value for each slip system group
- `tauSat_group_values` = $\tau_{Sat}$ value for each slip system group
- `hardeningExponent_group_values` $m$ value for each slip system group
- `selfHardening_group_values` $q^{\alpha\alpha}$
- `coplanarHardening_group_values` $q^{\alpha\beta}$ for co-planar slip systems
  in the same group
- `GroupGroup_Hardening_group_values` $q^{\alpha\beta}$ This parameter requires
  NxN values with N being the number of groups. Values are listed as $value_ij$
  (e.g. ij=11,12,21,22) with $i$ being the actual group and $j$ the secondary
  group. For $i=j$ the value represents the latent hardening coefficient
  between one slip system and all the non co-planar ones belonging the the same
  group. For $i\neq j$, the value represents the latent hardening coefficient
  between all the slip systems belonging to group $i$ and $j$. Note that,
  usually, the matrix associated with `GroupGroup_Hardening_group_values` is
  symmetric.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/crystal_plasticity/user_object_based/user_object_Voce_BCC.i block=UserObjects/state_var_evol_rate_comp_voce

In this example illustrate a BCC in which slip systems have been grouped by slip system class.

!syntax parameters /UserObjects/CrystalPlasticityStateVarRateComponentVoce

!syntax inputs /UserObjects/CrystalPlasticityStateVarRateComponentVoce

!syntax children /UserObjects/CrystalPlasticityStateVarRateComponentVoce
