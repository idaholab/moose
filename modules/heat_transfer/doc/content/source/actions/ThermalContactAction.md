# ThermalContactAction

This action sets up the models for [ThermalContact](syntax/ThermalContact/index.md). It supports the
default node-face formulation and a modular mortar formulation.

In mortar mode, the action creates the thermal Lagrange multiplier, modular gap conductance
constraint, and selected gap-flux model user objects. If a mechanical mortar `ContactAction`
generates a mortar mesh for an exactly matching primary/secondary boundary pair, the thermal action
reuses its lower-dimensional subdomains. Otherwise, the thermal action creates the lower-dimensional
subdomains needed for its mortar interface.
