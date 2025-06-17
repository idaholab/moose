# SCM model for the PNNL 12-pin benchmark

## Benchmark Description

The PNNL 2$\times$6 benchmark [!cite](BATES1980) was performed at Pacific Northwest National Laboratory in 1977 for investigating the buoyancy effect in the mixed (combined free and forced) convection regime for specific flow coast-down transients. The objective of the study was to develop an understanding of the thermal hydraulic phenomena at low flows in a pin bundle subjected to lateral power skews. The study was performed using a non-uniformly electrically heated 2x6 pin bundle contained in a flow housing.

Local fluid velocity and temperature measurements in the mixed (combined free and forced) convection regime were obtained. This benchmark aims to validate the SCM code in natural convection conditions. For each of the  test conditions of this study, fluid axial velocity and temperature (both local and bulk average inlet and outlet) were measured within the heated length of the pin bundle.  For the steady state cases studied, velocity measurements were made along the X axis at Y = 0.0 which is along the centerline of the central subchannels (subchannels 1-7). The cross section of the pin bundle is presented in Figure [2x6].

!media subchannel/v&v/pnnl/2x6.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=2x6
    caption=  Bundle Cross-Section with Subchannel Numbering Scheme and Bundle Dimensions (in.)

Velocity data were recorded at selected points along the central subchannel axial centerlines (centerpoints) for the transient flow cases.  Cases 5, 9 and 13 where chosen to test the SCM code. For the transient case 5, the location of the experimental measurements is the center of subchannel 4. The nominal run conditions for these cases are shown in Table [parameters].

!table id=parameters caption=Operational parameters for PNNL 2X6-pin benchmark.
| Case number | Initial Flow $[GPM]$  | Final Flow $[GPM]$ | time $[sec]$ | Power Gradient
|$Q_H [kW/pin]$ | $Q_L [kW/pin]$ | Re {initial/final} |
| :- | :- | :- | :- | :- | :- | :- | :- | :- |
| $5$ | $3.08$ | $1.08$ | $150$ | $0:0$ | $0.0$ | $0.0$ | $1200/420$ |
| $9$ | $3.08$ | $3.08$ | S.S | $1:0$ | $0.91$ | $0.0$ | $1290/1290$ |
| $13$ | $3.08$ | $3.08$ | S.S | $2:1$ | $0.91$ | $0.455$ | $1340/1340$ |

## SCM Input

### Steady State

The input file for the steady case 9 is:

!listing /validation/PNNL_12_pin/steady_state/2X6_ss.i language=moose

The input file decribing the radial pin power profile is:

!listing /validation/PNNL_12_pin/steady_state/power_profile.txt language=moose

For case 13 the input file and power profile file, need to be adapted according to the respective operational parameters.

### Transient

The input file for the steady case 5 is:

!listing /validation/PNNL_12_pin/transient/2X6_transient.i language=moose

## Results

The SCM results vs the experimental measurements are shown in Figures [buoyancy9], [buoyancy13] and [coast_down].

!media subchannel/v&v/pnnl/buoyancy9.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=buoyancy9
    caption=  Velocity profile across centerline

!media subchannel/v&v/pnnl/buoyancy13.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=buoyancy13
    caption=  Velocity profile across centerline

!media subchannel/v&v/pnnl/coast_down.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=coast_down
    caption=  Linear flow coast down

The code predicted average values in all three cases are lower than the measured experimental values. That's because the experimental results are pointwise instantaneous velocity measurements while the SCM results are the surface-averaged velocities in each subchannel.

The experimental maximum values measured at the subchannel centers are less than the analytically predicted value of $U_{max} = 2 \times U_{average}$ for laminar flow inside a circular pipe. Where $U_{average}$ is the surface average of the flow profile which is what SCM calculates.  This happens because of turbulence (modeled in SCM with closure models) which tends to flatten the velocity profiles. In turbulent flows momentum is transferred towards the wall regions, hence the maximum velocity in a turbulent profile is less than that in the laminar case.

Furthermore, when the power ratio is reduced (case 13 vs case 9), the velocity profile gets more flat. The code prediction follows that trend. Buoyancy effects are more pronounced when there are more extreme gradients in heat rate in the radial direction.

In all cases the SCM results follow the trend of the experimental measurements. Last it should be noted that the initial bump in the code prediction in Figure [coast_down] relates to the time required for the boundary condition information to reach the point of the measurement downstream.
