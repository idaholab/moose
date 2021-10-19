# GreaterThanLessThanPostprocessor

!syntax description /Postprocessors/GreaterThanLessThanPostprocessor

## Description

The `GreaterThanLessThanPostprocessor` computes the total number of degrees of
freedom of a nonlinear `variable` that are greater than or less than the
parameter `value`. The comparison of greater or less is specified using the
`MooseEnum` parameter `comparator`; the possible values of `comparator` are
`greater` or `less`. The computation may also be restricted to a subdomain using
the parameter `subdomain`.

## Example Syntax

!listing test/tests/nodalkernels/constraint_enforcement/lower-bound.i block=Postprocessors

!syntax parameters /Postprocessors/GreaterThanLessThanPostprocessor

!syntax inputs /Postprocessors/GreaterThanLessThanPostprocessor

!syntax children /Postprocessors/GreaterThanLessThanPostprocessor

!bibtex bibliography
