!! MOOSE Documentation Stub: Remove this line when content is added.

# Executioner

The Executioner controls the "execution" behavior of the simulation. MOOSE includes several Executioners but
most simulations will use the [Transient.md] Executioner, which is for simulating that is based on a PDE that
has a time dependence (changes over time).

## Attribute Reporting

The Executioner has a shortcut method that can be used to add an attribute reporting mechanism to the simulation.
The Executioner accomplishes this by adding a special [Postprocessor.md] to the simulation that will report
a potentially changing scalar value over time.
