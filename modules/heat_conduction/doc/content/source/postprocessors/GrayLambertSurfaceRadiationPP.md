# GrayLambertSurfaceRadiationPP

!syntax description /Postprocessors/GrayLambertSurfaceRadiationPP

## Description

This postprocessor extracts radiosity, heat flux density, or temperature from
the GrayLambertSurfaceRadiation userobject. The boundary from which this information
is extracted needs to be specified.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity.i
block=Postprocessors


!syntax parameters /Postprocessors/GrayLambertSurfaceRadiationPP

!syntax inputs /Postprocessors/GrayLambertSurfaceRadiationPP

!syntax children /Postprocessors/GrayLambertSurfaceRadiationPP

!bibtex bibliography
