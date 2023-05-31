# Subchannel model for the Thermal-Hydraulic Out-of-Reactor Safety Six-Channel Center Blockage Benchmark

## Benchmark Description

THORS bundle 3A also simulates the Fast Flux Test Facility and Clinch River Breeder Reactor configurations. Nineteen electrically heated pins are contained inside a round duct, which has unheated dummy pins along the duct wall. The central six channels (1, 2, 3, 4, 5 and 6) are blocked by a non-heat-generating 6.35-mm-thick stainless-steel plate [!cite](fontana1973effect). The bundle cross section is shown in [fig:thors]. The circles with the crosses indicate the position of thermocouples at the assembly exit. Pronghorn-SC modeled the THORS bundle 3A blockage with a 90% area reduction on the affected subchannels. The Pronghorn-SC model's geometry and subchannel and rod index notation is shown in [fig:hex_index]. The experimental parameters are presented in [parameters].

!media figures/thors.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors
    caption= THORS bundle 3A cross section.

!media figures/hex-index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:hex_index
    caption= Pronghorn-SC model cross-section of THORS bundle and index notation \\ (white: fuel pin index; black: subchannel index; red: gap index).

!table id=parameters caption=Design and operational parameters for THORS six-channel blockage benchmark.
| Experiment Parameter (unit) | Value |
| :- | :- |
| Number of pins (---) | $19$ |
| Rod pitch (cm) | $0.726$ |
| Rod diameter (cm) | $0.584$ |
| Wire wrap diameter (cm) | $0.142$ |
| Wire wrap axial pitch (cm) | $30.48$ |
| Flat-to-flat duct distance (cm) | $3.41$ |
| Inlet length (cm) | $30.48$ |
| Heated length (cm) | $53.34$ |
| Outlet length (cm) | $7.62$  |
| Blockage location (cm) | $68.58$ |
| Outlet pressure (Pa) | $2.0 \times 10^{5}$ |
| Inlet temperature (K) | $714.261$ |
| Inlet flow rate (m$^3$/s) | $3.41 \times 10^{-3}$ |
| Power profile (---) | Uniform |
| Pin power (kW/m) | $33$ |

 Run 101 was chosen to validate Pronghorn-SC performance. The THORS experiment measured the temperatures at the exits of selected subchannels. Due to the approximation of the circular experimental test section with a hexagonal Pronghorn-SC model, there is a subchannel index correspondence between the two geometries as follows: 43(37), 42(36), 17(20), 16(10), 3(4), 6(1), 8(14) and 28(28). Where the number outside the parentheses refers to the Pronghorn-SC model and the number inside the parentheses refers to the experimental.

## Results

Figure [fig:thors_val] presents the exit temperature distribution, expressed as $T-T_{in}$ for the experimental case of 33 kW/m per pin and 100% flow at 54 gpm (Run~101), along with the Pronghorn-SC prediction. Predicted subchannel average temperatures agreed relatively well with measured experimental values for Run 101 with a six-channel center blockage with a bigger error in Subchannel 17(20). Nevertheless, Pronghorn-SC consistently over-predicted the exit temperature for all subchannels, which can be attributed to Pronghorn-SC's smaller cross-sectional area. It should be noted that Pronghorn-SC calculates surface averages while the experimental results are measured at the subchannel centers. As such, it is expected that Pronghorn-SC results will be a bit higher than the experimental values, since the location of the measurements is away from the heated walls in the center of the subchannels. The discrepancy in Subchannel 17(20) might very well be attributed to the location of the thermocouples and the approximate relationship between the model and actual experiment geometry. For the center subchannels where the Pronghorn-SC model geometry is more representative, the agreement is better. The poorer agreement in the exterior subchannels may be due to steeper temperature gradients in that region since the Pronghorn-SC code calculates average channel temperatures, whereas the thermocouples might be in a subchannel temperature gradient.

!media figures/FFM-3A.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors_val
    caption= Exit temperature profile.

A CFD model was developed to use as a reference solution and to further evaluate Pronghorn-SC's performance. The CFD model had about 1 million cells and utilized an implicit unsteady transient solver. Segregated fluid and energy solvers, $k-\omega$ turbulence modeling and the default polyhedral STAR-CCM+ mesher were used. [fig:CFD-vector],[fig:CFD-velocity] and [fig:CFD-temperature] present the CFD simulation results on a 2D plane around the blockage location.

!media figures/vector_field.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:CFD-vector
    caption= CFD velocity vector field.

!media figures/velocity_field.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:CFD-velocity
    caption= CFD average axial velocity field.

!media figures/temperature_field.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:CFD-temperature
    caption= CFD average temperature field.

Furthermore, the axial profiles of massflow and temperature are plotted for a center subchannel along the stream-wise direction in [fig:FFM-3A_massflow] and [fig:FFM-3A_temperature], respectively. Massflow is forced around the blockage, which causes flow to be reduced in the axial direction. The blockage causes a recirculation region to be formed downstream, which can been seen in CFD results.  Due to the axial flow being reduced around the area of the blockage, a temperature peak is observed. On the other hand, downstream of the blockage, recirculation causes a cooling effect, as massflow rushes back in the central region from the outer cooler subchannels, causing the temperature to drop back down. The axial profile of the average temperature of the center subchannel agrees well with the CFD calculation of the center-line temperature. Before the blockage, the average value is a bit higher than the center value. After the blockage, enhanced mixing causes the values to overlap. Using the Pronghorn-SC temperature profile, a broad estimation of the recirculation length can be made by measuring the distance between the end of the heating pick and the end of the blockage. The result is 1.765 inches, which is consistent with the experimentally reported 2 inches [!cite](fontana1973effect).

!media figures/FFM-3A_massflow.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-3A_massflow
    caption= Axial profile of mass flow in center subchannel.

!media figures/FFM-3A_T.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-3A_temperature
    caption= Axial profile of temperature in center subchannel.

It should be noted that the effect of the simulated blockage, depends on the axial discretization, flow area reduction and user-defined local form loss coefficient, along with the turbulent modeling parameters. Due to the nonlinear nature of the friction pressure drop calculation, the effect of these parameters is not straightforward and special care must be taken by the user to properly simulate the blockage effects and produce consistent results.

## Subchannel Inputs

The input file to run the blockage case (Run~101) is presented below:

!listing /examples/Blockage/FFM-3A.i language=cpp

The file that creates the detailed mesh that subchannel solution gets projected on is presented below:

!listing /examples/Blockage/FFM-3Adetailed.i language=cpp
