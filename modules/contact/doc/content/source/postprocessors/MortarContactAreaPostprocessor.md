# MortarContactAreaPostprocessor

`MortarContactAreaPostprocessor` computes the surface area in contact by leveraging
similar techniques to [VolumePostprocessor.md](VolumePostprocessor), but only
integrating over elements that have a Lagrange Multiplier variable value above
a given threshold.

# Description and Syntax

!syntax description /Postprocessors/MortarContactAreaPostprocessor

!syntax parameters /Postprocessors/MortarContactAreaPostprocessor

!syntax inputs /Postprocessors/MortarContactAreaPostprocessor

!syntax children /Postprocessors/MortarContactAreaPostprocessor
