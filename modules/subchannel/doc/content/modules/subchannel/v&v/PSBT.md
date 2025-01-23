# Steady-state mixing model Validation

The PSBT 5x5 benchmark is an international benchmark developed by the Organisation for Economic Co-operation and Development (OECD), the Nuclear Regulatory Commission (NRC), and the Nuclear Power Engineering Center(NUPEC) [!cite](PSBT1). In this work we utilize the steady-state mixing test, detailed in volume III of the PSBT benchmark [!cite](PSBT3). The purpose of this test is to validate the mixing models of the participating codes. The participating codes predict the fluid temperature distribution at the exit of the heated section of a pin bundle assembly and compare it with the experimental values provided by the benchmark. Here we will present as an example case 01-5237, as well as the average error for all the cases run [!cite](kyriakopoulos2022development). The pin bundle geometry description is presented in Table below.

| PSBT pin bundle specifications |
| |

| Item | Value |
| - | - |
| Rods array | $5\times5$ |
| Number of heated pins | $25$ |
| Heated pin outer diameter (mm) | $9.50$ |
| Pitch (mm) | $12.60$ |
| Axial heated length (mm) | $3658$ |
| Flow channel inner width (mm) | $64.9$  |
| Axial power shape | Uniform  |
| Number of mixing vaned (MV) spacers | $7$  |
| Number of non mixing vaned (NMV) spacers | $2$  |
| Number of simple spacers (SS) | $8$  |
| MV spacer location (mm) | $457,914,1372,1829,2286,2743,3200$ |
| NMV spacer location (mm) | $0,3658$  |
| Simple spacer location (mm) | $229,686,1143,1600,2057,2515,2972,3429$ |

The pin bundle has a radial power profile in which the right side of the assembly is under-heated. The radial power profile is shown in [fig:Power]. The pins on the left side transfer 100% of available pin power to the fluid, while the pins on the right transfer 25%. This causes an uneven temperature distribution at the exit of the assembly.

!media subchannel/v&v/Radial_Power.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:Power
    caption=Radial pin power profile

Note that the turbulent mixing parameter used for all the cases was: $β = 0.08$, which differs significantly from the default value of $β = 0.006$. The reason behind the need to adjust $β$ to a much higher value, has to do with the geometry of the PSBT facility. Note that there is a preferential mixing direction of the experimental results in the diagonal direction (towards one corner of the assembly),
exhibited in [fig:profile], while the code results for both values of $β$ are symmetric as expected. The experimental results exhibit a non symmetric distribution that we cannot capture with a constant value of beta. There is a temperature gradient towards the corner due to an additional mixing effect, which may reduce the exit temperature differences between the two regions (hot/cold, left/right) and finally increase the optimum $β$ in comparison with the case of no temperature gradient in each region.

!media subchannel/v&v/01-5237.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:profile
    caption=Exit temperature distribution for case 01-5237

The reason for the additional mixing effect is thought to be the special mixing vanes that the NUPEC facility incorporates in its design. Specifically, the temperature gradient appearing in the experimental data was attributed to the thermal mixing in the diagonal direction of the test bundle, which may be caused by the alignment of mixing vanes mounted in the spacer grids [!cite](hwang2012accuracy). This is the physical reason behind the need to use an increased value of $\beta = 0.08$.

This illustrates the fact that modeling parameters like $\beta$ should ideally be calibrated for specific geometries and in no way can they be applied generally without proper justification. Nevertheless our results show that, provided we choose the optimum parameters adjusted for the specific geometries, we can accurately predict the exit temperatures. Figure [fig:MixingError] presents the cumulative mean absolute error in the exit temperature in comparison with other subchannel codes [!cite](PSBT3). We note that for the temperature mixing test, our code performs adequately in comparison to the other codes. Our mean absolute error is calculated to be $Error = 2.176$ which places us in 5th place out of the nine codes.

!media subchannel/v&v/psbt-results.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:MixingError
    caption=Mean absolute error in predicted exit temperature

## Input file for case: 01-5237

!listing /examples/psbt/psbt_ss/psbt_example.i language=cpp