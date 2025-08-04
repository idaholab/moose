# SCMQuadPowerIC

!syntax description /ICs/SCMQuadPowerIC

## Overview

!! Intentional comment to provide extra spacing

This IC assigns the axial heat rate ([!param](/ICs/SCMQuadPowerIC/variable) = `q_prime`) on the subchannels or pins in the case of a problem with subchannels/pins in a
square lattice arrangement. The user must provide the  total power of the subassembly [!param](/ICs/SCMQuadPowerIC/power), the axial shape of the power profile and the radial
power distribution (power per pin). The axial power profile is given as a function over the -z direction, which integral over the length of the heated portion of the pin, is equal
to the length of the heated portion of the pin. The radial power distribution is given as a column of numbers in a .txt file [!param](/ICs/SCMQuadPowerIC/filename) that has as many entries as the number of pins.

If the first entry is 1.0, that means that the pin with index 0 is at 100% power. If the 5th entry has a value of 0.0 that means that pin with index 4 has 0% power, etc.
The pin and subchannel indexes are presented in [user notes page](user_notes.md).

A pin with 100% power has a value of power which is equal to the total power of the subassembly divided with the total number of heated pins (if a pin has zero power it doesn't count in that number).

The total power of pin with index 6 is calculated as the product of the 100% pin power, times the value on the 7th line of the radial power distribution .txt file. Hence the user
should pay attention that the sum of entries on the radial power distribution file should be equal to the number of heated pins.

The axial heat rate at a specific height is the product of the total power of the pin, times the value of the axial power profile function, at that height.

## Caveat

!! Intentional comment to provide extra spacing

If the user has created a mesh for the pins, the axial heat rate will be assigned to the nodes of the pin mesh. If the user hasn't created a pin mesh the appropriate heat rate will be assigned to
the nodes of the subchannel mesh.

!syntax parameters /ICs/SCMQuadPowerIC

!syntax inputs /ICs/SCMQuadPowerIC

!syntax children /ICs/SCMQuadPowerIC
