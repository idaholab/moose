# DepletionIDGenerator

!syntax description /Mesh/DepletionIDGenerator

## Overview

`DepletionIDGenerator` object generates and assigns depletion IDs (extra element integers) on a mesh by finding unique combinations of reporting and material IDs.
For a pin-level depletion case, the individual pins can be identified by the pin and assembly IDs, and the detailed depletion regions within a pin can be further divided by material IDs.
For assembly-wise depletion, the user can set up the depletion IDs by combining the assembly and material IDs.
Once the detailed depletion regions are determined, depletion IDs are uniquely assigned to those resulting regions.

## Depletion ID Control

[!param](/Mesh/DepletionIDGenerator/id_name) lists extra element integer ID names used for setting up the depletion zones.
Note that the material ID does not need to be defined in the list because it is included by default.
A depletion zone (elements with the same depletion id) is always covered by one material region (elements with the same material id).
The level of details in depletion zones can be controlled by the extra element integer IDs defined in [!param](/Mesh/DepletionIDGenerator/id_name).
Any different combination of extra element integer IDs is considered a unique depletion zone.

For example, one can set up pin-by-pin and axial layer-by-layer arrangement of depletion zone by specifying those three reporting IDs: pin, assembly and plane IDs.

An alterantive extra element integer ID name for the material ID can be optionally specified in [!param](/Mesh/DepletionIDGenerator/material_id_name). Its default value is set to  material_id.

A list of extra element integer ID can be provided in [!param](/Mesh/DepletionIDGenerator/exclude_id_name) and [!param](/Mesh/DepletionIDGenerator/exclude_id_value) to exclude certain domains from being assigned depletion IDs.
For those domains specified in [!param](/Mesh/DepletionIDGenerator/exclude_id_name) and [!param](/Mesh/DepletionIDGenerator/exclude_id_value), the depletion ID is set to zero because the depletion ID must be assigned for every mesh element even if not used in the actual depletion calculation.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/reporting_id/depletion_id/depletion_id.i block=Mesh

!syntax parameters /Mesh/DepletionIDGenerator

!syntax inputs /Mesh/DepletionIDGenerator

!syntax children /Mesh/DepletionIDGenerator
