# QA tests of PorousFlow

The capability of PorousFlow is rigorously tested through a large number of regression tests
that are provided in the test suite. Many of these tests are designed to recover analytical
solutions to the problem or to reproduce well-known benchmark problems. The pages below describe some of
the more non-trivial tests.

- [Infiltration and drainage](tests/infiltration_and_drainage/infiltration_and_drainage_tests.md)
- [Heat and fluid responses in finite 1D bars subject to various boundary conditions](tests/newton_cooling/newton_cooling_tests.md)
- [Poroelasticity](tests/poro_elasticity/poro_elasticity_tests.md)
- [Boundary sinks and sources](tests/sinks/sinks_tests.md)
- [Point and line sources, pumping tests, boreholes](tests/dirackernels/dirackernels_tests.md)
- [Multi-phase, multi-component radial injection, using fluidstate](tests/fluidstate/fluidstate_tests.md)
- [Injection ala Buckley and Leverett](tests/buckley_leverett/buckley_leverett_tests.md)
- [Evolution of a pressure pulse](tests/pressure_pulse/pressure_pulse_tests.md)
- [Heat conduction](tests/heat_conduction/heat_conduction_tests.md)
- [Heat advection](tests/heat_advection/heat_advection_tests.md)
- [Fluid mass computation and conservation](tests/mass_conservation/mass_conservation_tests.md)
- [Establishment of gravitational head](tests/gravity/gravity_tests.md)
- [Dispersion and diffusion](tests/dispersion/dispersion_tests.md)
- [Heating from inelastic deformation](tests/plastic_heating/plastic_heating_tests.md)
- [1D heat and mass transport](tests/avdonin/1d_avdonin.md)
- [1D radial heat and mass transport](tests/avdonin/1d_radial_avdonin.md)
- [Tests of the Jacobian](tests/jacobian/jacobian_tests.md)

Many of the PorousFlow tests were created before MOOSE's current documentation system was established.  Therefore, we are still in the process of documenting the tests.  The following pages need to be completed.

- [actions](tests/actions/actions_tests.md)
- [fluids](tests/fluids/fluids_tests.md)
- [numerical_diffusion](tests/numerical_diffusion/numerical_diffusion_tests.md)
- [adv_diff_bcs](tests/adv_diff_bcs/adv_diff_bcs_tests.md)
- [aux_kernels](tests/aux_kernels/aux_kernels_tests.md)
- [flux_limited_TVD_advection](tests/flux_limited_TVD_advection/flux_limited_TVD_advection_tests.md)
- [basic_advection](tests/basic_advection/basic_advection_tests.md)
- [flux_limited_TVD_pflow](tests/flux_limited_TVD_pflow/flux_limited_TVD_pflow_tests.md)
- [poroperm](tests/poroperm/poroperm_tests.md)
- [functions](tests/functions/functions_tests.md)
- [radioactive_decay](tests/radioactive_decay/radioactive_decay_tests.md)
- [capillary_pressure](tests/capillary_pressure/capillary_pressure_tests.md)
- [relperm](tests/relperm/relperm_tests.md)
- [chemistry](tests/chemistry/chemistry_tests.md)
- [density](tests/density/density_tests.md)
- [heterogeneous_materials](tests/heterogeneous_materials/heterogeneous_materials_tests.md)
- [desorption](tests/desorption/desorption_tests.md)
- [thermal_conductivity](tests/thermal_conductivity/thermal_conductivity_tests.md)
- [thm_rehbinder](tests/thm_rehbinder/thm_rehbinder_tests.md)
- [energy_conservation](tests/energy_conservation/energy_conservation_tests.md)
- [non_thermal_equilibrium](tests/non_thermal_equilibrium/non_thermal_equilibrium_tests.md)
