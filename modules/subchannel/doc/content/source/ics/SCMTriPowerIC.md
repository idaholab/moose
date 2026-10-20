# SCMTriPowerIC

!syntax description /ICs/SCMTriPowerIC

## Overview

!! Intentional comment to provide extra spacing

This IC assigns the axial heat rate ([!param](/ICs/SCMTriPowerIC/variable) = `q_prime`) to the fuel pins in a triangular lattice arrangement.

The user must provide the total power of the subassembly [!param](/ICs/SCMTriPowerIC/power), the axial shape of the power profile and the radial
power distribution (power per pin). The axial power profile is given as a function [!param](/ICs/SCMTriPowerIC/axial_heat_rate) of the distance from the start of the heated portion of the pin. The radial power distribution is given as a column of numbers in a .txt file [!param](/ICs/SCMTriPowerIC/filename) that has as many entries as the number of pins.

If the first entry is 1.0, that means that the pin with index 0 is at 100% power. If the 5th entry has a value of 0.0 that means that pin with index 4 has 0% power, etc.
The pin and subchannel indexes are presented in [user notes page](user_notes.md).

The radial power distribution is normalized by the sum of its entries, so the pin powers always add up to [!param](/ICs/SCMTriPowerIC/power). A pin with 100% power has a value of power which is equal to the total power of the subassembly divided by the sum of the entries. If the entries sum to the number of heated pins, a pin with 100% power has the average pin power.

The total power of pin with index 6 is calculated as the product of the 100% pin power, times the value on the 7th line of the radial power distribution .txt file.

The axial heat rate at a specific height is the total power of the pin divided by the heated length, times the value of the axial power profile function at that height, times a correction factor for that pin. The correction factor is computed once, at initial setup, so that the trapezoidal integral of the nodal axial heat rate over the pin, which is the integral the subchannel solver uses, equals the total power of the pin. The axial power profile function therefore only sets the shape of the profile. If its integral over the heated length equals the heated length, the correction factor is close to 1. The correction factor is computed with the function evaluated at $x = y = 0$, while the nodal axial heat rate uses the actual node coordinates, so the function should depend only on $z$ (and time); otherwise the pin powers no longer match the radial power distribution. Nodes outside the heated portion of the pin are assigned zero axial heat rate.

## Caveat

!! Intentional comment to provide extra spacing

This IC requires a pin mesh. The axial heat rate ([!param](/ICs/SCMTriPowerIC/variable) = `q_prime`) is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error. The subchannel solver distributes this pin heat to neighboring subchannels when computing the heat added to the coolant.

!syntax parameters /ICs/SCMTriPowerIC

!syntax inputs /ICs/SCMTriPowerIC

!syntax children /ICs/SCMTriPowerIC
