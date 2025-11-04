# GradDiv

This class implements an augmented Lagrange term, whose weak form is given by $\int \gamma \nabla \cdot \vec{u} \nabla \psi$ where $\gamma$ is the stabilization parameter, $\vec{u}$ is the velocity, and $\psi$ corresponds to the velocity test function. For more details about grad-div stabilization please read [INSADMomentumGradDiv.md]. The difference between this class and that one is that this class is meant to be used with scalar field component velocity variables while `INSADMomentumGradDiv` is meant for use with the vector variable implementation for Taylor-Hood elements.

!syntax parameters /Kernels/GradDiv

!syntax inputs /Kernels/GradDiv

!syntax children /Kernels/GradDiv
