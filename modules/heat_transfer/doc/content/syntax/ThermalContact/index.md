# Thermal Contact

The `ThermalContact` syntax models heat transfer between paired surfaces. The `formulation`
parameter selects either the default `node_face` formulation or the `mortar` formulation.

## Node-face formulation

The default `node_face` formulation uses a node-on-face approach and preserves the existing
`ThermalContact` behavior. The `type` parameter selects the thermal contact model.

### Gap offset parameters

Gap offset can be provided to the node-face formulation via the `PenetrationAux`. It can be either
`secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset
from primary side but mapped to secondary side). Use of these gap offset parameters treats the
surfaces as if they were virtually extended (positive offset value) or narrowed (negative offset
value) by the specified amount, so that the surfaces are treated as if they are closer or further
away than they actually are. There is no resistance to heat transfer within the material in this gap
offset region.

### Multiple contact pairs

Users may need to set up thermal contact between multiple contact pairs. For that application, users
can provide arrays of primary and secondary boundary names which will match consecutively to define
thermal contact pairs. The same thermal contact-related input parameters will be applied to all
contact pairs defined in the action input.

!listing test/tests/multiple_contact_pairs/multiple_contact_pairs.i block=ThermalContact

## Mortar formulation

The `mortar` formulation uses a thermal Lagrange multiplier and the modular gap-flux models used by
[MortarGapHeatTransfer](syntax/MortarGapHeatTransfer/index.md). Set `formulation = mortar` and provide
one `primary` boundary, one `secondary` boundary, and the temperature `variable`. Mortar mode
supports one primary/secondary pair per `ThermalContact` block.

Built-in conduction and radiation models are selected with `gap_flux_options`. Alternatively,
user-created gap-flux model user objects can be supplied with `user_created_gap_flux_models`.
Parameters such as `thermal_lm_scaling`, `correct_edge_dropping`, the modular conduction and
radiation parameters, and the mortar gap-geometry parameters configure the generated objects. The
node-face `type` parameter is not used in mortar mode.

When a mechanical `Contact` action using a mortar formulation generates a mortar mesh for the same
primary and secondary boundaries in the same orientation, `ThermalContact` reuses that action's
lower-dimensional mortar subdomains. This avoids generating duplicate lower-dimensional elements
for coupled thermomechanical contact. Reversed primary and secondary roles are not treated as a
match. A `Contact` action with `generate_mortar_mesh = false` is not reused. If there is no matching
mechanical mortar action, `ThermalContact` creates its own lower-dimensional subdomains.

!syntax parameters /ThermalContact
