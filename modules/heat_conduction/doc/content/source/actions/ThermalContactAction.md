# Thermal Contact Action

## Description

The `ThermalContactAction` action sets up the set of models used
to enforce thermal contact across two surfaces. See the description,
example use, and parameters on the
[ThermalContact](/ThermalContact/index.md) system documentation page.

### Gap offset parameters

Gap offset can be provided to the current contact formulation via the `PenetrationAux`. It can be either `secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset from primary side but mapped to secondary side). However, the offsetted gap is treated as rigid region without temperature gradient.
