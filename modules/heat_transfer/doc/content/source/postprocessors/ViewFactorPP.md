# ViewFactorPP

!syntax description /Postprocessors/ViewFactorPP

## Description

This postprocessor extracts view factors between `from_boundary` to `to_boundary` from the view factor
userobject provided by `view_factor_object_name`.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity_automatic_vf.i
block=Postprocessors


!syntax parameters /Postprocessors/ViewFactorPP

!syntax inputs /Postprocessors/ViewFactorPP

!syntax children /Postprocessors/ViewFactorPP

!bibtex bibliography
