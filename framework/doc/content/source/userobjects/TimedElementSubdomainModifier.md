# TimedElementSubdomainModifier

!syntax description /UserObjects/TimedElementSubdomainModifier

## Overview

The `TimedElementSubdomainModifier` is a base class to be inherited by other user objects. It provides 
functionality to change the element subdomain only at a given list of times and handles the corresponding

- Moving boundary/interface nodeset/sideset modification,
- Solution initialization, and
- Stateful material property initialization,

all of which are demonstrated in the [CoupledVarThresholdElementSubdomainModifier.md].

This base class is inherited by

- [TimedSubdomainModifier.md]

!syntax parameters /UserObjects/TimedElementSubdomainModifier

!syntax inputs /UserObjects/TimedElementSubdomainModifier

!syntax children /UserObjects/TimedElementSubdomainModifier
