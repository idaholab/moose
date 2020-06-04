# Thermal Contact Action

## Description

The `ThermalContactAction` action sets up the set of models used
to enforce thermal contact across two surfaces. See the description,
example use, and parameters on the
[ThermalContact](/ThermalContact/index.md) system documentation page.

### Gap offset parameters

Gap offset can be provided to the current contact formulation via the `PenetrationAux`. It can be either `slave_gap_offset` (gap offset from slave side) or `mapped_master_gap_offset` (gap offset from master side but mapped to slave side). However, the offsetted gap is treated as rigid region without temperature gradient.
