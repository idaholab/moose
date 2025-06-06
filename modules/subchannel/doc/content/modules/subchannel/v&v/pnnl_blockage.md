# SCM model for the Pacific Northwest Laboratory's (PNNL) $7 \times 7$ Sleeve Blockage Benchmark

## Benchmark Description

PNL's $7 \times 7$ sleeve blockage facility was designed to investigate the turbulent flow phenomena near postulated sleeve blockages in a model nuclear fuel pin bundle. The sleeve blockages were characteristic of fuel-clad `swelling` or `ballooning`, which could occur during loss-of-coolant accidents in pressurized-water reactors [!cite](creer1976effects). The experimental parameters are presented in [parameters].

!table id=parameters caption=Design and operational parameters for PNL's $7\times7$ sleeve blockage benchmark.
| Experiment Parameter (unit) | Value |
| :- | :- |
| Number of pins (---) | $7 \times 7$ |
| Rod pitch (cm) | $1.36906$ |
| Rod diameter (cm) | $0.99568$ |
| Length (cm) | $144.78$ |
| Outlet pressure (Pa) | $101325$ |
| Inlet temperature (K) | $302.594$ |
| Reynolds number (---) | $2.9 * 10^4$ |
| Inlet velocity (m/sec) | $1.73736$ |
| Power profile (---) | Uniform zero power |
| Grid spacer location (cm) | $40.64,  142.24$ |
| Grid spacer loss coefficient (---) | $1.14, 1.14$ |
| Sleeve blockage location (cm) | $64.135$ (midway between the spacers) |

Sleeve blockages (three inches in length) were positioned on the center nine pins of the bundle. Area reductions, of 70% and 90%, were obtained in the center four subchannels of the bundle. The 70% and 90% blockages corresponded to area reductions of 35% and 45% in the subchannels adjacent to the sides of the cluster and 17% and 22% in the subchannels next to the corners of the blockage, respectively. These area reductions were not intended to define those expected during loss-of-coolant accidents but were chosen to provide a severe test case to verify subchannel computer programs. Axial components of local mean velocity and intensity of turbulence were measured, using a one-velocity component laser Doppler anemometer.

## Results

The 70% and 90% blockage was chosen to validate SCM performance. SCM models the blockage by decreasing the surface area of the affected subchannels accordingly. Since the subchannel formulation is based on the concept of the hydraulic diameter, reducing the surface area affects the system of equations in multiple ways. Most significantly through the $Re$ number and the friction model, pressure drop calculation. Restricting the flow area increases the pressure drop and causes flow to diverge to the adjacent subchannels. Furthermore, the user has the option to apply a concentrated form loss coefficient on the affected subchannels at the corresponding axial cell. This will have an  effect similar to the area reduction. SCM was run with 28 axial cells for the 70% blockage case and 84 axial cells for the 90% blockage case. A user-set local form loss coefficient at the blockage, $K_{bl} = 0.3$ and $K_{bl} = 0.9$, was also applied for the two cases, respectively, which was axially distributed among the blocked cells. These values were fitted to produce the best agreement. The default modeling parameters $C_T=2.6, \beta = 0.006$ were also used. In addition to SCM, a CFD simulation (Star-CCM+) of the experiment was made with 10.5 million cells, for the 70% blockage case. The results presented in [fig:70blockage] and [fig:90blockage] showcase the relative velocity of a center subchannel across the length of the assembly.

!media subchannel/v&v/pnnl/70BlockageDefault.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:70blockage
    caption= The 70% sleeve blockage.

!media subchannel/v&v/pnnl/90BlockageDefault.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:90blockage
    caption= The 90% sleeve blockage.

SCM utilized the implicit monolithic solver, specifically developed to deal with recirculation, as it was the only one that managed to robustly solve the problem. Predicted subchannel average velocities agreed well with measured values for both cases. SCM's predicted flow of a central subchannel over-predicts the mixing length downstream of the blockage and is quicker to reduce upstream of the blockage. One possible explanation of this behavior has to do with the nature of the SCM calculations. Averaged quantities over relatively large volumes are expected to be slower to adapt to local rapid changes. This could also indicate that the inter-channel mixing is underestimated.

For this reason, the SCM imulation was run again, this time with a larger turbulent mixing parameter of $\beta = 0.06, 36~axial~cells, K_{bl} = 1$ and the result is presented in Figure [fig:70blockage2]. The simulation with the adjusted mixing effect agrees much better with the experimental results, especially downstream of the blockage. This suggests that the calibrated  default value of $\beta$ is not general enough to adequately model scenarios where a blockage augments the mixing effects in the wake.

At the exit region of the blockage, the experimental velocity profile obtained with the 70% blockage exhibits a jetting characteristic that was not measured in the 90% blockage case. According to the authors of the experimental analysis [!cite](creer1976effects), jetting may not have been detected with the 90% blockage because the measuring volume could not be positioned as close to the blockage axial center line as was possible with the 70% blockage. Though it is also probable that no jetting was present due to flow choking. SCM overestimates the jetting effect in both cases.

!media subchannel/v&v/pnnl/70blockage.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:70blockage2
    caption= The 70% sleeve blockage with $\beta = 0.06$.

It should be noted that the CFD simulation took about 3 hours to converge, while SCM took about 3 seconds. Considering that, along with the agreement of the SCM results with the experimental data, one can say that the SCM is a useful engineering tool for modeling blockage scenarios in square water-cooled assemblies. The effect of the simulated blockage, depends on the axial discretization, flow area reduction and user-defined local form loss coefficient, along with the turbulent modeling parameters. Due to the nonlinear nature of the friction pressure drop calculation, the effect of these parameters is not straightforward and special care must be taken by the user to properly simulate the blockage effects and produce consistent results.

## SCM Inputs

The input file to run the 70% blockage case with the `improved` parameters is presented below:

!listing /validation/Blockage/PNNL_7x7/7X7blockage70.i language=moose

The file that creates the detailed mesh that subchannel solution gets projected on is presented below:

!listing /validation/Blockage/PNNL_7x7/detailedMesh.i language=moose
