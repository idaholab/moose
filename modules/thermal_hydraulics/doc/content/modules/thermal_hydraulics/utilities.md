# THM Utilities

THM provides the following [Peacock](python/peacock.md optional=True) plugins, located in
`modules/thermal_hydraulics/python/peacock/`, relative to the root MOOSE directory:

- `FluidPropertyInterrogatorPlugin.py`: Computes various fluid properties from a
  [FluidProperties](syntax/FluidProperties/index.md) object. Note that currently there
  is no way to specify non-default input parameter values, and those objects
  requiring input parameters to be specified cannot be used.
- `UnitConverterPlugin.py`: Converts between various units of measurement.
- `ModelBuilder/FlowChannelParametersCalculator.py`: Computes cross-sectional
  area, hydraulic area, and wetted perimeter for various flow channel geometries.

While these are considered Peacock plugins, you do not actually run these within
an instance of Peacock but instead, run their source files directly with `python`.
For example,

```
cd ~/projects/moose/modules/thermal_hydraulics/python/peacock/
python FluidPropertyInterrogatorPlugin.py
```
