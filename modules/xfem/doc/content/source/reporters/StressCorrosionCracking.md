# StressCorrosionCracking

## Description

This Reporter computes the growth increment for each active crack front node in the [CrackMeshCut3DUserObject.md].  The growth increment for each node is computed by first finding the crack front node where $K_I$ provided by the [DomainIntegralAction.md] is largest and computing the amount of time, $t_{cmax}$, it will take for that node to extend by the amount given by [!param](/Reporters/StressCorrosionCracking/max_growth_size).  $t_{cmax}$ is stored in the scalar reporter [!param](/Reporters/StressCorrosionCracking/time_to_max_growth_size_name).  The growth increment for the other nodes in the crack front are then computed by multiplying their growth rates by $t_{cmax}$.  The growth rate comptuted by `StressCorrosionCracking` is based on the equation for stress corrosion cracking given by [!cite](li_scc_2015) for stainless steel 316L exposed to water at 288C given by

\begin{equation}
v_c =
\begin{cases}
2 \times 10^{-9}, & K_I \leq 6.7 \ \text{MPa} \sqrt{\text{m}} \\
3.33 \times 10^{-11} K_I^{2.161}, & 6.7 \ \text{MPa} \sqrt{\text{m}} \leq K_I \leq 59 \ \text{MPa} \sqrt{\text{m}} \\
2.01 \times 10^{-7}, & K_I \geq 59 \ \text{MPa} \sqrt{\text{m}}
\end{cases}
\label{eqn:scc_ss316}
\end{equation}

which is input into `StressCorrosionCracking` using the following input file syntax:

```text
[Reporters]
  [scc_crack_growth]
    type = StressCorrosionCracking
    growth_increment_name = "growth_increment"
    time_to_max_growth_size_name = "max_growth_timestep"
    crackMeshCut3DUserObject_name = cut_mesh
    max_growth_size = 0.0004
    k_low = 6.7e6
    growth_rate_low = 2e-9
    k_high = 59e6
    growth_rate_high = 2.01e-7
    growth_rate_mid_multiplier = 3.601e-24 #3.33e-11*(1e-6)**2.161
    growth_rate_mid_exp_factor = 2.161
  []
[]
```

## Example Syntax

This example from the test suite demonstrates the time to grow the crack by [!param](/Reporters/StressCorrosionCracking/max_growth_size) is the same as that produced using a constant timestep size.

!listing /modules/xfem/test/tests/solid_mechanics_basic/edge_crack_3d_scc_crit.i block=Reporters

!syntax parameters /Reporters/StressCorrosionCracking

!syntax inputs /Reporters/StressCorrosionCracking

!syntax children /Reporters/StressCorrosionCracking

!bibtex bibliography
