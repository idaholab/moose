# MixedModeEquivalentK
!syntax description /Postprocessors/MixedModeEquivalentK

## Description
This object computes an equivalent mixed-mode stress intensity factor, $K_{eq}$, from the Mode $I$, $II$, and $III$ stress intensity factors $K_I$, $K_{II}$, and $K_{III}$, respectively:

$$
K_{eq} = \sqrt{K_I K_{II} K_{III} + \frac{1}{(1-\nu) * K_{III}^2}}
$$

!syntax parameters /Postprocessors/MixedModeEquivalentK

!syntax inputs /Postprocessors/MixedModeEquivalentK

!syntax children /Postprocessors/MixedModeEquivalentK
