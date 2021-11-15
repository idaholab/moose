# Using Stochastic Tools with Multiphysics Models

The purpose of this document is to present a multiphysics example using the [Stochastic Tools Module](modules/stochastic_tools/index.md). The intention is to showcase the capabilities of the module to produce statistically relevant results including uncertainty propagation and sensitivity, as well as the module's surrogate modeling infrastructure. The problem of interest is a thermomechanics model using a combination of the [Heat Conduction](modules/heat_conduction/index.md) and [Tensor Mechanics](modules/tensor_mechanics/index.md) modules. The problem involves multiple uncertain material properties and multiple quantities of interest (QoI). Using both Monte Carlo sampling and polynomial chaos surrogate modeling, the effect of these properties' uncertainties are quantified with uncertainty propagation and global sensitivity analysis.

## Problem Description

The problem of interest involves a steady-state thermomechanics model. The geometry is a 3-D finite hollow cylinder with two concentric layers of different material properties, seen in [fig:geom]. Due to symmetry, only 1/8 of the cylinder is represented by the mesh. The inner surface of the ring is exposed to a surface heat source that has a cosine shape along the axis of the cylinder with its peak being at the center. The end and outside of the cylinder have convective boundary conditions. The cylinder is free to displace in all directions due to the thermal expansion. The relevant material and geometric properties are listed in [tab:mat_prop]. This table also lists the "uncertain" parameters that will be described in the following section. For reference, the temperature and displacement profiles are shown in [fig:sol_temp] to [fig:sol_dispz] where the uncertain properties are set to some arbitrary values. The full input file for this model is shown in [list:thermo].

!media combined/geom.png caption=Model Problem Geometry id=fig:geom style=width:50%

!table caption=Material Properties for Thermomechanics Cylinder id=tab:mat_prop
| Property | Symbol | Value | Units  |
| :- | :- | -: | -: |
| Half Cyliner Height | $L$ | 3 | m |
| Inner Radius | $R$ | 1.0 | m |
| Inner Ring Width | $r_1$ | 0.1 | m |
| Outer Ring Width | $r_2$ | 0.1 | m |
| Outer Heat Transfer Coef. | $h_{\mathrm{outer}}$ | 10 | W/m$^2\cdot$K |
| Outer Free Temperature | $T_{\infty,\mathrm{outer}}$ | 300 | K |
| End Heat Transfer Coef. | $h_{\mathrm{end}}$ | 10 | W/m$^2\cdot$K |
| End Free Temperature | $T_{\infty,\mathrm{end}}$ | 300 | K |
| Heat Source Magnitude | $Q_t$ | Uncertain | W |
| Inner Thermal Conductivity | $k_1$ | Uncertain | W/m$\cdot$K |
| Outer Thermal Conductivity | $k_2$ | Uncertain | W/m$\cdot$K |
| Inner Young's Modulus | $Y_1$ | Uncertain | Pa |
| Outer Young's Modulus | $Y_2$ | Uncertain | Pa |
| Inner Poisson's Ratio | $\nu_1$ | Uncertain |  |
| Outer Poisson's Ratio | $\nu_2$ | Uncertain |  |
| Inner Thermal Expansion Coef. | $\alpha_1$ | Uncertain | 1/K |
| Outer Thermal Expansion Coef. | $\alpha_2$ | Uncertain | 1/K |
| At Rest Temperature | $T_0$ | 300 | K |

!row!

!media combined/temp.png caption=Temperature (K) id=fig:sol_temp style=width:33%;float:left

!media combined/disp.png caption=Displacement (m) id=fig:sol_disp style=width:33%;float:left

!media combined/dispx.png caption=X-Displacement (m) id=fig:sol_dispx style=width:33%;float:left

!media combined/dispy.png caption=Y-Displacement (m) id=fig:sol_dispy style=width:33%;float:left

!media combined/dispz.png caption=Z-Displacement (m) id=fig:sol_dispz style=width:33%;float:left

!row-end!

!listing examples/stochastic/graphite_ring_thermomechanics.i caption=Thermomechanics model input file id=list:thermo

### Uncertain Parameters

A total of nine properties of the model are not known exactly, but with some known probability of values occurring. The probabilities are represented by each parameter's probability density function. All parameters have a independent uniform distribution, $\mathcal{U}(a,b)$, where $a$ and $b$ are the lower and upper limit values for the property, respectively. [tab:uprop] lists the details of each of the property's distribution.

!table caption=Uniform distribution parameters for uncertain properties id=tab:uprop
| Index | Property | $a$ | $b$ |
| :- | :- | - | - |
| 1 | $k_1$ | 20 | 30 |
| 2 | $k_2$ | 90 | 110 |
| 3 | $Q_t$ | 9000 | 11000  |
| 4 | $\alpha_1$ | 1.0$\times10^{-6}$ | 3.0$\times10^{-6}$ |
| 5 | $\alpha_2$ | 0.5$\times10^{-6}$ | 1.5$\times10^{-6}$ |
| 6 | $Y_1$ | 2.0$\times10^5$ | 2.2$\times10^5$ |
| 7 | $Y_2$ | 3.0$\times10^5$ | 3.2$\times10^5$ |
| 8 | $\nu_1$ | 0.29 | 0.31 |
| 9 | $\nu_2$ | 0.19 | 0.21 |

### Quantities of Interest

There are a total of ten QoIs for the model, which involve temperature and displacement:

1. Temperature at center of inner surface --- $T_{1,c}$
1. Temperature at center of outer surface --- $T_{2,c}$
1. Temperature at end of inner surface --- $T_{1,e}$
1. Temperature at end of outer surface --- $T_{2,e}$
1. x-displacement at center of inner surface --- $\delta_{x,1,c}$
1. x-displacement at center of outer surface --- $\delta_{x,2,c}$
1. x-displacement at end of inner surface --- $\delta_{x,1,e}$
1. x-displacement at end of outer surface --- $\delta_{x,2,e}$
1. z-displacement at end of inner surface --- $\delta_{z,1}$
1. z-displacement at end of outer surface --- $\delta_{z,2}$


[fig:qoi] shows geometrically where these QoIs are located in the model.

!media combined/qoi.png caption=Geometric description of quantities of interest id=fig:qoi style=width:75%

## Results

In this exercise, we will use the [statistics](Statistics.md) and [Sobol sensitivity](PolynomialChaosReporter.md) capabilities available in the stochastic tools module. The goal of this exercise is to understand how the uncertainty in the parameters affects the the resulting QoIs. This is done through sampling the model at different perturbations of the parameters and performing statistical calculations on resulting QoI values. Two methods are used to perform this analysis. First is using the sampler system to perturb the uncertain properties and retrieve the QoIs which will undergo the analysis. The second is training a [polynomial chaos surrogate](PolynomialChaos.md) and using that reduced order model to sample and perform the analysis. The idea is that many evaluations of the model are necessary to compute accurate statistical quantities and surrogate modeling speeds up this computation by requiring much fewer full model evaluations for training and is significantly faster to evaluate once trained.

Using [latin hypercube sampling](LatinHypercubeSampler.md), the thermomechanics model was run with a total of 100,000 samples, the input file is shown by [list:lhs]. A order four polynomial chaos surrogate was training using a Smolyak sparse quadrature for a total of 7,344 runs of the full model. The training input is shown by [list:train] and the evaluation input is shown by [list:eval]. [tab:rt] shows the run-time for sampling the full order model and training and evaluating the surrogate. We see here that cumulative time for training and evaluating the surrogate is much smaller than just sampling the full order model, this is because building the surrogate required far fewer evaluations of the full model and evaluating the surrogate is much faster than evaluating the full model.

!table caption=Stochastic run-time results id=tab:rt
| Simulation | Samples | CPU Time |
| :- | :- | :- |
| Full-Order Sampling | 100,000 | 176 hr  |
| Polynomial Chaos --- Training | 7,344 | 13.7 hr  |
| Polynomial Chaos --- Evaluation | 100,000 | 6.8 s  |

!listing combined/examples/stochastic/lhs_uniform.i id=list:lhs caption=Latin hypercube sampling and statistics input file

!listing combined/examples/stochastic/poly_chaos_train_uniform.i id=list:train caption=Polynomial chaos training input file

!listing combined/examples/stochastic/poly_chaos_uniform.i id=list:eval caption=Polynomial chaos evaluation input file

### Statistics

[tab:stat] shows the statistical results of sampling the thermomechanis model and the polynomial chaos surrogate. $\mu$ and $\sigma$ represent the mean and standard deviation of the QoI, and CI is the confidence interval. Note that the confidence interval for the polynomial chaos statistics is not relevant since these values were found analytically using integration techniques. [fig:hist_temp] to [fig:hist_dispz] compares several of the probability distributions of the QoIs between sampling the full-order model and the polynomial chaos surrogate.

!table caption=Statistics Results id=tab:stat
| QoI | $\mu$ | 95% CI | $\sigma$ | 95% CI | PC -- $\mu$ | PC -- $\sigma$ |
| :- | - | - | - | - | - | - |
| $T_{1,c}$        | 609.97    | (609.87, \\ 610.06)       | 18.22     | (18.18, \\ 18.27)         | 609.97    | 18.23 |
| $T_{2,c}$        | 586.85    | (586.77, \\ 586.94)       | 16.64     | (16.60, \\ 16.68)         | 586.85    | 16.64 |
| $T_{1,e}$        | 506.31    | (506.25, \\ 506.37)       | 12.05     | (12.03, \\ 12.08)         | 506.31    | 12.05 |
| $T_{2,e}$        | 507.92    | (507.85, \\ 507.98)       | 12.13     | (12.10, \\ 12.15)         | 507.92    | 12.12 |
| $\delta_{x,1,c}$ | 4.032E-04 | (4.028E-04, \\ 4.037E-04) | 8.608E-05 | (8.581E-05, \\ 8.636E-05) | 4.032E-04 | 8.625E-05 |
| $\delta_{x,2,c}$ | 4.996E-04 | (4.990E-04, \\ 5.001E-04) | 1.078E-04 | (1.075E-04, \\ 1.082E-04) | 4.996E-04 | 1.081E-04 |
| $\delta_{x,1,e}$ | 4.220E-04 | (4.213E-04, \\ 4.226E-04) | 1.255E-04 | (1.252E-04, \\ 1.258E-04) | 4.220E-04 | 1.256E-04 |
| $\delta_{x,2,e}$ | 4.793E-04 | (4.786E-04, \\ 4.800E-04) | 1.352E-04 | (1.349E-04, \\ 1.356E-04) | 4.794E-04 | 1.354E-04 |
| $\delta_{z,1}$   | 6.139E-04 | (6.131E-04, \\ 6.146E-04) | 1.466E-04 | (1.462E-04, \\ 1.470E-04) | 6.139E-04 | 1.469E-04 |
| $\delta_{z,2}$   | 4.995E-04 | (4.989E-04, \\ 5.000E-04) | 1.067E-04 | (1.065E-04, \\ 1.072E-04) | 4.995E-04 | 1.071E-04 |

!row!

!media combined/temp_center_inner_hist.png caption=$T_{1,c}$ Histogram id=fig:hist_temp style=width:50%;float:left

!media combined/dispx_center_inner_hist.png caption=$\delta_{x,1,c}$ Histogram id=fig:hist_dispx style=width:50%;float:left

!media combined/dispz_inner_hist.png caption=$\delta_{z,1}$ Histogram id=fig:hist_dispz style=width:50%;float:left

!row-end!

### Sobol Sensitivities

Sobol sensitivities, or Sobol indicies, are a metric to compare the global sensitivity a parameter has on a QoI. This examples demonstrates several different types of the sensitivities. The first is total sensitivity, which measure the total sensitivity from a parameter, [fig:sobol_total] shows these values for each QoI and parameter. The second is a correlative sensitivity, which measures the sensitivity due to a combination of parameters, [fig:heat_temp] to [fig:heat_dispz] show heat maps of these values for several QoIs.

!media combined/sobol_total.png caption=Total Sobol sensitivities id=fig:sobol_total style=width:100%

!row!

!media combined/temp_center_inner_sobol.png caption=$T_{1,c}$ Sobol sensitivities id=fig:heat_temp style=width:50%;float:left

!media combined/dispx_center_inner_sobol.png caption=$\delta_{x,1,c}$ Sobol sensitivities id=fig:heat_dispx style=width:50%;float:left

!media combined/dispz_inner_sobol.png caption=$\delta_{z,1}$ Sobol sensitivities id=fig:heat_dispz style=width:50%;float:left

!row-end!

## Supplementary Figures

### Probablity Distributions for All QoIs

!gallery! large=6
!card combined/temp_center_inner_hist.png title=$T_{1,c}$

!card combined/temp_center_outer_hist.png title=$T_{2,c}$

!card combined/temp_end_inner_hist.png title=$T_{1,e}$

!card combined/temp_end_outer_hist.png title=$T_{2,e}$

!card combined/dispx_center_inner_hist.png title=$\delta_{x,1,c}$

!card combined/dispx_center_outer_hist.png title=$\delta_{x,2,c}$

!card combined/dispx_end_inner_hist.png title=$\delta_{x,1,e}$

!card combined/dispx_end_outer_hist.png title=$\delta_{x,2,e}$

!card combined/dispz_inner_hist.png title=$\delta_{z,1}$

!card combined/dispz_outer_hist.png title=$\delta_{z,2}$
!gallery-end!

### Sobol Sensitivity Heatmaps for All QoIs

!gallery! large=6
!card combined/temp_center_inner_sobol.png title=$T_{1,c}$

!card combined/temp_center_outer_sobol.png title=$T_{2,c}$

!card combined/temp_end_inner_sobol.png title=$T_{1,e}$

!card combined/temp_end_outer_sobol.png title=$T_{2,e}$

!card combined/dispx_center_inner_sobol.png title=$\delta_{x,1,c}$

!card combined/dispx_center_outer_sobol.png title=$\delta_{x,2,c}$

!card combined/dispx_end_inner_sobol.png title=$\delta_{x,1,e}$

!card combined/dispx_end_outer_sobol.png title=$\delta_{x,2,e}$

!card combined/dispz_inner_sobol.png title=$\delta_{z,1}$

!card combined/dispz_outer_sobol.png title=$\delta_{z,2}$
!gallery-end!
