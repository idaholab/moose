# Thermal Contact Action

## Description

The `ThermalContactAction` action sets up the set of models used to enforce thermal contact across two surfaces using a node on face approach. See the description, example use, and parameters on the [ThermalContact](syntax/ThermalContact/index.md) system documentation page.

### Gap offset parameters

Gap offset can be provided to the current contact formulation via the `PenetrationAux`. It can be either `secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset from primary side but mapped to secondary side). Use of these gap offset parameters treats the surfaces as if they were virtually extended (positive offset value) or narrowed (negative offset value) by the specified amount, so that the surfaces are treated as if they are closer or further away than they actually are. There is no  resistance to heat transfer within the material in this gap offset region.

### Multiple contact pairs (node/face)

Users may need to set up thermal contact between multiple contact pairs. For that application, users can provide arrays of primary and secondary boundary names which will match consecutively to define thermal contact pairs. The same thermal contact-related input parameters will be applied to all contact pairs defined in the action input. 

!listing test/tests/multiple_contact_pairs/multiple_contact_pairs.i block=ThermalContact   