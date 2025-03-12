# Friction model Verification

## Test Description

&nbsp;

This verification problem is the same used in [!cite](CTF-Verification). This case presents a problem where the effects of friction are clearly discernible and quantifiable. Momentum transfer in the single-phase case is driven by a lateral pressure gradient and turbulence. By deactivating turbulence in the test model case, momentum transfer can only be the result of lateral pressure imbalance; which for a model with no form losses (spacer grids), can only be driven by unequal frictional losses. Friction loss depends on the hydraulic diameter, so it makes sense to devise a two-channel problem, with channels that have an unequal flow area. The problem geometry is shown in [fig-friction].

!media subchannel/v&v/friction.png
    style=width:30%;margin-bottom:2%;margin:auto;
    id=fig-friction
    caption=Friction model verification problem geometry

Channel-2 has a hydraulic diameter that is twice the size of the Channel-1 hydraulic diameter. The length of the model is set to 10 m to allow the flow to completely redistribute within the solution space. The different frictional pressure drops create a lateral pressure gradient that drives flow from the higher resistance channel to the lower resistance channel. Moving up the channels, velocity grows larger in the low-resistance channel, which increases frictional pressure drop in that channel. Simultaneously, velocity decreases in the high-resistance channel, which decreases frictional pressure drop. This continues until the frictional pressure drop is the same in both channels, at which point crossflow ceases. At this point, the channels are said to be in mechanical equilibrium. An analytical solution can be derived for this point of mechanical equilibrium:

\begin{equation}
\dot{m}_{in} = \dot{m}_2 \bigg( 1 + \bigg(  \frac{D_{h,2}}{D_{h,1}}\bigg)^{\frac{C_2 - 1}{C_2 + 2}} \frac{A_1}{A_2}\bigg).
\end{equation}

## Results

&nbsp;

The analytical prediction is compared with the code results in [friction-ver]. The code results converge to the analytical solution at the mechanical equilibrium.

!media subchannel/v&v/friction-ver.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=friction-ver
    caption=Relative mass flow distribution in the axial direction

## Input file

!listing /examples/verification/friction_model_verification/two_channel.i language=cpp