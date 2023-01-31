# Multiphysics

!---

# Defining Grid Variables

!row!
!col! width=45%
```
[AuxVariables]
  [tf]
    initial_condition = 350
  []
  [tAl]
    initial_condition = 350
  []
  [bu]
    initial_condition = 0
  []
[]
```
!col-end!

!col! width=10%
!!
!col-end!

!col! width=45%
```
[GlobalParams]
  library_file = XS_research_reactor.xml
  library_name = serpent_pbr_xs

  grid_names = 'tf tAl bu'
  grid_variables = 'tf tAl bu'
  isotopes = pseudo
  densities = 1.0

  is_meter = true
[]

[Materials]
  [water]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'water'
    material_id = 3
  []
  [clad]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'clad'
    material_id = 2
  []
  [fuel]
    type = CoupledFeedbackNeutronicsMaterial
    block = 'fuel'
    material_id = 1
    plus = true
  []
[]
```
!col-end!
!row-end!

!---

# Power Density

!equation
p = P \frac{\sum_{g=1}^{G} \kappa^{g} \Sigma_{f}^g \phi^g}{\int_{\mathcal{D}}\sum_{g=1}^{G} \kappa^{g} \Sigma_{f}^g \phi^g\,dx}

!equation
\begin{aligned}
p \equiv& \text{Power density} \\
P \equiv& \text{User-specified power} \\
\kappa \equiv& \text{Energy released per fission} \\
\Sigma_f \equiv& \text{Fission cross section}
\end{aligned}

```
[PowerDensity]
  power = 60e6 # 60 MW
  power_density_variable = power_density
[]
```

!---

# MultiApps and Transfers

```
[MultiApps]
  [heat_conduction]
    type = FullSolveMultiApp
    input = heat_conduction.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [power_density]
    type = MultiAppCopyTransfer
    to_multi_app = heat_conduction
    source_variable = power_density
    variable = power_density
  []
  [fuel_temperature]
    type = MultiAppInterpolationTransfer
    from_multi_app = heat_conduction
    source_variable = Tfuel
    variable = tf
  []
  [clad_temperature]
    type = MultiAppInterpolationTransfer
    from_multi_app = heat_conduction
    source_variable = Tclad
    variable = tAl
  []
[]
```

!---

# DFEM-SN Multiphysics

!media cmfd_multiphysics.png

- Performing iteration with multiphysics fits naturally with sweeper-CMFD iteration
- No changes to the `Executioner` block are needed
- Having `fixed_point_max_its > 1` will instigate the multiphysics-CMFD inner iteration

  - We've seen that doing this causes divergent behavior

!---

# CFEM-Diffusion Multiphysics

```
[Executioner]
  type = Eigenvalue/Transient
  ...
  fixed_point_max_its = 100
  fixed_point_rel_tol = 1e-6
  fixed_point_abs_tol = 1e-8

  custom_pp = eigenvalue
[]
```

- To perform multiphysics iteration with CFEM-Diffusion, the `Executioner` block +must+ have `fixed_point_max_its>1`
- To use postprocessor quantities (like $k_{\mathrm{eff}}$) for convergence check, use the `custom_pp` parameter
