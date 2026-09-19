# Grain Boundary Shifted Cohesive Zone Test

These tests solve a shifted cohesive zone problem on a polycrystalline grain structure using two
supported methods for providing the true grain interfaces:

- One test supplies a single saved surface mesh containing the complete boundary of each grain.
- The other test supplies a separate saved surface mesh for each grain interface.

In both cases, the shifted cohesive zone action supplies the interface distances and normals to the
non-AD bilinear mixed-mode traction model.
