# Effect of Partial Blockages in Simulated LMFBR Fuel Assemblies

Information on the THORS facility and experiments can be found in the following sources: [!cite](fontana1973effect),[!cite](han1977blockages),
[!cite](jeong2005modeling).

## Central blockage of 6 channels in a 19-pin sodium-cooled bundle

THORS bundle 3A simulates the Fast Flux Test Facility and Clinch River Breeder Reactor configurations.  Nineteen electrically heated pins are contained inside a round duct, which has unheated dummy pins along the duct wall. The central six channels ($1, 2, 3, 4, 5, 6$) are blocked by a non-heat-generating 35-mm-thick stainless-steel plate. The bundle cross section is shown in [fig:thors]. The circles with the crosses indicate the position of thermocouples at the assembly exit. SCM modeled the THORS bundle 3A blockage with a $92$% area reduction on the affected subchannels and a local form loss coefficient of $6$. The SCM model's geometry and subchannel/rod index notation is shown in [fig:hex_index]. The experimental parameters are presented in [parameters].

!media figures/thors.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors
    caption= THORS bundle 3A cross section.

!media figures/hex-index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:hex_index
    caption= SubChannel model cross-section of THORS bundle and index notation \\ (white: fuel pin index; black: subchannel index; red: gap index).

!table id=parameters caption=Design and operational parameters for THORS six-channel blockage benchmark.
| Experiment Parameter (unit) | Value |
| :- | :- |
| Number of pins (---) | $19$ |
| Rod pitch (cm) | $0.726$ |
| Rod diameter (cm) | $0.5842$ |
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

 Run~$101$ was chosen to validate SCM performance. The THORS experiment measured the temperatures at the exit of selected subchannels. There is a subchannel index correspondence between the experiment and the model, as follows: 43(37), 42(36), 17(20), 16(10), 3(4), 6(1), 8(14) and 28(28). Where the number outside the parentheses refers to the SCM model and the number inside the parentheses refers to the experimental convention. SCM has currently no capablility to model triangular assemblies within circular ducts. Hence, the experimental circular duct is approximated with a hexagonal duct.

## Results for central blockage

Figure [fig:thors_val] presents the exit temperature distribution, expressed as $T-T_{in}$ along with the SCM calculation. For this case power was at $33kW/m$ per pin and $100$% flow at $54 gpm$. Predicted subchannel average temperatures agreed relatively well, with a bigger error in Subchannel 17(20).  It should be noted that SCM calculates surface averages while the experimental results are measured at the subchannel centers. As such, it is expected that SCM results will be a bit higher than the experimental values, since the location of the measurements is away from the heated rod walls. The discrepancy in Subchannel 17(20) might very well be attributed to the location of the thermocouples and the approximate relationship between the model and actual experiment geometry. For the center subchannels where the SCM model geometry is more representative, the agreement is better. The poorer agreement in the exterior subchannels may be due to steeper temperature gradients in that region since SCM calculates average channel temperatures, whereas the thermocouples might be in a subchannel temperature gradient.

!media figures/FFM-3A.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors_val
    caption= Exit temperature profile ($C_T = 1$).

Thus far, the turbulent modeling parameter $C_T$ has been calibrated only for square lattice, bare fuel pin assemblies. As such, in the above example $C_T$ was arbitarily set to be equal to $1$. As a reminder $C_T$ is a tuning parameter that affect turbulent momentum cross flows. Lerger values of $C_T$ will lead to higher cross flows. Higher $C_T$ means more turbulent momentum mixing and flatter velocity profiles. For $C_T = 10$ the code calculation is presented in Figure [fig:thors_val2]. This calculation presents a better agreement with the experimental results, which suggests that the presence of a blockage induces mixing.

!media figures/FFM-3A2.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors_val2
    caption= Exit temperature profile ($C_T = 10$).

A CFD model was developed to use as a reference solution and to further evaluate SCM's performance. The CFD model had about 1 million cells and utilized an implicit unsteady transient solver. Segregated fluid and energy solvers, $k-\omega$ turbulence modeling and the default polyhedral STAR-CCM+ mesher were used. [fig:CFD-vector],[fig:CFD-velocity] and [fig:CFD-temperature] present the CFD simulation results on a 2D plane around the blockage location.

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

Furthermore, the axial profiles of massflow and temperature are plotted for a center subchannel along the stream-wise direction in [fig:FFM-3A_massflow] and [fig:FFM-3A_temperature], respectively. Massflow is forced around the blockage, which causes flow to be reduced in the axial direction. The blockage causes a recirculation region to be formed downstream, which can been seen in CFD results.  Due to the axial flow being reduced around the area of the blockage, a temperature peak is observed. On the other hand, downstream of the blockage, recirculation causes a cooling effect, as massflow rushes back in the central region from the outer cooler subchannels, causing the temperature to drop back down. The axial profile of the average temperature of the center subchannel agrees well with the CFD calculation of the center-line temperature. Before the blockage, the average value is a bit higher than the center value. After the blockage, enhanced mixing causes the values to overlap. Using the SCM temperature profile, a broad estimation of the recirculation length can be made by measuring the distance between the end of the heating pick and the end of the blockage. The result is 1.765 inches, which is consistent with the experimentally reported 2 inches [!cite](fontana1973effect).

!media figures/FFM-3A_massflow.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-3A_massflow
    caption= Axial profile of mass flow in center subchannel.

!media figures/FFM-3A_T.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-3A_temperature
    caption= Axial profile of temperature in center subchannel.

It should be noted that the effect of the simulated blockage, depends on the axial discretization, flow area reduction and user-defined local form loss coefficient, along with the turbulent modeling parameters. Due to the nonlinear nature of the friction pressure drop calculation, the effect of these parameters is not straightforward and special care must be taken by the user to properly simulate the blockage effects and produce consistent results.

## SCM Inputs

The input file to run the central blockage case is presented below:

!listing /examples/Blockage/FFM-3A.i language=cpp

The file that creates the detailed mesh that subchannel solution gets projected on is presented below:

!listing /examples/Blockage/FFM-3Adetailed.i language=cpp

## Edge blockage of 14 channels in 19-pin sodium-cooled bundles

THORS bundle 5B has the same fuel configuration as bundle 2B, except that 0.0711-cm-diam wire-wrap spacers are used to separate the peripheral pins from the duct wall. The half-size spacers are used to reduce the flow in the peripheral flow channels and to cause a flatter radial temperature profile across the bundle. It also means that the flat-to-flat distance is reduced appropiately. The pins have a heated length of $45.7 cm$. A 3175-cm-thick stainless steel blockage plate is located $10.2 cm$  above the start of the heated zone to block $14$ edge and internal channels along the duct wall. The test section layout is shown in Fig [fig:thors2]. The experimental parameters for the chosen case are presented in [parameters2]. SCM modeled the THORS bundle 5B blockage with a $92$% area reduction on the affected subchannels and a local form loss coefficient of $6$. $C_T$ was set to $10$ as in the previous case. The SCM model's geometry and subchannel/rod index notation is shown in [fig:hex_index].

!media figures/thors2.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:thors2
    caption= THORS bundle 5B cross section.

!table id=parameters2 caption=Design and operational parameters for THORS 14-channel edge blockage benchmark.
| Experiment Parameter (unit) | Value |
| :- | :- |
| Number of pins (---) | $19$ |
| Rod pitch (cm) | $0.726$ |
| Rod diameter (cm) | $0.5842$ |
| Wire wrap diameter (cm) | $0.1422$ |
| Wire wrap axial pitch (cm) | $30.5$ |
| Flat-to-flat duct distance (cm) | $3.241$ |
| Inlet length (cm) | $40.64$ |
| Heated length (cm) | $45.72$ |
| Outlet length (cm) | $15.24$  |
| Blockage location (cm) | $95.96$ |
| Outlet pressure (Pa) | $2.0 \times 10^{5}$ |
| Inlet temperature (K) | $596.75/541.55$ |
| Inlet velocity (m/s) | $6.93/0.48$ |
| Power profile (---) | Uniform |
| Power (kW) | $145/52.8$ |

## Results for edge blockage

The case presented here is the high flow case (FFM Series 6, Test 12, Run 101). The thermocouples are located at the middle of the exit region. There is a subchannel index correspondence between the Figure [fig:thors2] and the Pronhorn-SC model shown in Figure[fig:hex_index] as follows: 34(39), 33(38), 18(20), 9(19), 3(4), 0(1), 12(11) and 25(30). Where the number outside the parentheses refers to the SCM model and the number inside the parentheses, refers to the experimental convention. SCM calculation along with the experimental measurements is shown in Figure [fig:FFM-5B]. The code calculations excibits generally good agreement with the experimental measurements. The least agreement occurs at the edge subchannels ($34,33$) which is likely due to the model not accuretally replicating the flow area there. Prongohorn-SC uses an assembly-wide constant wire diameter, while in the experimental assembly the wires at the edge subchannels had half the diameter.

!media figures/FFM-5B.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-5B
    caption= Exit temperature profile for high flow case ($C_T = 10$).

The second case presented here is the low flow case (FFM Series 6, Test 12, Run 109). The thermocouples are located at the middle of the exit region, same as before. SCM calculation along with the experimental measurements is shown in Figure [fig:FFM-5B2]. The code calculations excibits good agreement with the experimental measurements.

!media figures/FFM-5B2.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:FFM-5B2
    caption= Exit temperature profile for low flow case ($C_T = 10$).

## SCM Input

The input file to run the edge blockage case is presented below:

!listing /examples/Blockage/FFM-5B.i language=cpp
