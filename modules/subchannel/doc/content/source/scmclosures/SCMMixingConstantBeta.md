# SCMMixingConstantBeta

!syntax description /SCMClosures/SCMMixingConstantBeta

## Overview

!! Intentional comment to provide extra spacing

This closure class is used when the user wants to define a constant mixing coefficient $\beta$ for the whole subchannel assembly.

### Calibrated parameter values

!! Intentional comment to provide extra spacing

$\beta$ has been calibrated for quadrilateral assemblies using data from the 2x3 air-water facility that was operated by Kumamoto university. The purpose of that facility was to quantify the effects of mixing and void drift [!cite](SADATOMI). In these experiments, the turbulent mixing rates and the fluctuations of static pressure difference between subchannels were measured. The author derived a way to use the die concentration measurements, in order to calculate  the turbulent mixing rates ($w_{ij}'$) between subchannels [!cite](SADATOMI2). Additional Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).

It is important to note that the mixing coefficient is simply a tuning parameter that will depend on the specific geometry of the facility being modeled. This facility is a square lattice, but the geometry is much larger than that of a typical PWR pin-lattice geometry. Nevertheless this study is useful for showing that the code is capable of predicting the correct mixing rate if it is calibrated correctly.

After calibrating the turbulent diffusion coefficient $\beta$ we turned our attention to the turbulent modeling parameter $C_{T}$. This is a tuning parameter that informs on how much momentum is transferred/diffused between subchannels, due to turbulence. The CNEN 4x4 test [!cite](Marinelli) performed at Studsvik laboratory for studying the flow mixing effect between adjacent subchannels was chosen to tune this parameter. This experiment consists in velocity and temperature measurements taken at the outlet of a 16-pin assembly test section. Analysis of the velocity distribution at the exit of the assembly can be used to calibrate the turbulent parameter $C_{T}$. Additional Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

For quadrilateral assemblies, the calibration values computed were: $C_{T} = 2.6$, $\beta = 0.006$ [!cite](kyriakopoulos2022development).

!syntax parameters /SCMClosures/SCMMixingConstantBeta

!syntax inputs /SCMClosures/SCMMixingConstantBeta

!syntax children /SCMClosures/SCMMixingConstantBeta

!bibtex bibliography
