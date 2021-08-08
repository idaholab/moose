# Flow through fractured porous media

PorousFlow can be used to simulate flow (both fluid and heat) through a porous medium that contains explicit fractures.  Two methods are explained:

- [The fractures can be incorporated into the porous-media mesh](/porous_flow/nomultiapp_flow_through_fractured_media.md).  This is the preferred method, since fully-implicit time-stepping with PorousFlow's full Jacobian calculations provides unconditional stability and rapid convergence.
- [The fracture network is so complicated it must be meshed separately from the porous-media mesh](/porous_flow/multiapp_fracture_flow_introduction.md).  This method uses MultiApps and typically suffers from inferior numerical performance compared with the previous, but is frequently the only practical method.
