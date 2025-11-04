# External Fluid Properties Submodules

Additional submodules are available as `git submodules`. They can be downloaded by performing:

```
cd ~/project/moose
git submodule update --init --checkout modules/fluid_properties/contrib/air modules/fluid_properties/contrib/carbon_dioxide modules/fluid_properties/contrib/helium modules/fluid_properties/contrib/nitrogen modules/fluid_properties/contrib/potassium modules/fluid_properties/contrib/sodium
cd modules fluid_properties
make -j6
```

This will download and build the optional submodules.
The optional submodules contain fluid properties for:

- [Air](AirSBTLFluidProperties.md optional=True)
- Carbon dioxide [liquid](CarbonDioxideLiquidFluidProperties.md optional=True), [vapor](CarbonDioxideVaporFluidProperties.md optional=True) and [two-phase](CarbonDioxideTwoPhaseFluidProperties.md optional=True) fluid properties, as well with [the homogeneous equilibrium model](CarbonDioxideHEMFluidProperties.md optional=True)
- [Helium](HeliumSBTLFluidProperties.md optional=True)
- [Nitrogen](NitrogenSBTLFluidProperties.md optional=True)
- Potassium [liquid](PotassiumLiquidFluidProperties.md optional=True), [vapor](PotassiumVaporFluidProperties.md optional=True) and [two-phase](PotassiumTwoPhaseFluidProperties.md optional=True)
- Sodium [liquid](SodiumLiquidFluidProperties.md optional=True), [vapor](SodiumVaporFluidProperties.md optional=True) and [two-phase](SodiumTwoPhaseFluidProperties.md optional=True)