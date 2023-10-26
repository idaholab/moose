# Coupled Physics Tutorials

The following tutorial and example problems demonstrate how capabilities from multiple
physics modules can be used together:

- [Intoduction to Coupled Thermo-Mechanical Modeling](combined/tutorials/introduction/index.md)
- [Using Stochastic Tools with Multiphysics Models](combined/examples/stm_thermomechanics.md)

# Solid Isotropic Material Penalization (SIMP) Topology Optimization

While core optimization algorithms are available in the optimization module, the examples here
combine the use of such algorithms with mechanical and thermal physics. In particular, we
use SIMP to optimize single and multiple material problems. Additionally, we can optimize for
various conditions, whether they be various loads acting on a single system or various physics
acting on the same mesh domain.

All the following examples employ SIMP topology optimization (see, e.g., [!cite](sigmund200199)):

- [Two-dimensional mechanical bridge with radial average filtering](combined/examples/optimization/2d_mbb.md)
- [Two-dimensional mechanical bridge with Helmholtz, PDE filtering](combined/examples/optimization/2d_mbb_pde.md)
- [Two-dimensional mechanical bridge with adaptive mesh refinement](combined/examples/optimization/2d_mbb_pde_amr.md)
- [Three-dimensional mechanical bridge with radial average filtering](combined/examples/optimization/3d_mbb.md)
- [Multi-load with multiapp](combined/examples/optimization/multiload.md)
- [Thermal and mechanical optimization](combined/examples/optimization/thermomechanical.md)
- [Multimaterial](combined/examples/optimization/multimaterial.md)
