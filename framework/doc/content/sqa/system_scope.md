[!ac](MOOSE)'s scope is to provide a set of interfaces for building [!ac](FEM)
simulations. Abstractions to all underlying libraries are provided.

Solving coupled problems where competing physical phenomena impact one and other in a significant
nonlinear fashion represents a serious challenge to several solution strategies. Small perturbations
in strongly-coupled parameters often have very large adverse effects on convergence behavior. These
adverse effects are compounded as additional physics are added to a model. To overcome these
challenges, [!ac](MOOSE) employs three distinct yet compatible systems for solving these types of
problems.

First, an advanced numerical technique called the [!ac](JFNK) method is
employed to solve the most fully-coupled physics in an accurate, consistent way. An example of this
would be the effect of temperature on the expansion or contraction of a material. While the
[!ac](JFNK) numerical method is very effective at solving fully-coupled equations, it can also be
computationally expensive. Plus, not all physical phenomena in a given model are truly coupled to one
another. For instance, in a reactor, the speed of the coolant flow may not have any direct effect on
the complex chemical reactions taking place inside the fuel rods.  We call such models
"loosely-coupled". A robust, scalable system must strike the proper balance between the various
modeling strategies to avoid performing unnecessary computations or incorrectly predicting behavior
in situations such as these.

[!ac](MOOSE)'s Multiapp system will allow modelers to group physics into logical categories where
[!ac](MOOSE) can solve some groups fully-coupled and others loosely-coupled. The Multiapp system
goes even further by also supporting a "tightly-coupled" strategy, which falls somewhere between the
"fully-coupled" and "loosely-coupled" approaches. Several sets of physics can then be linked together
into logical hierarchies using any one of these coupling strategies, allowing for several potential
solution strategies. For instance, a complex nuclear reactor model might consist of several
tightly-coupled systems of fully-coupled equations.

Finally, [!ac](MOOSE)'s Transfers system ties all of the physics groups contained within the
Multiapp system together and allows for full control over the flow of information among the various
groups. This capability bridges physical phenomena from several different complementary scales
simultaneously. When these three [!ac](MOOSE) systems are combined, myriad coupling combinations
are possible. In all cases, the [!ac](MOOSE) framework handles the parallel communication, input,
output and execution of the underlying simulation. By handling these computer science tasks, the
[!ac](MOOSE) framework keeps modelers focused on doing research.

[!ac](MOOSE) innovates by building advanced simulation capabilities on top of the very best
available software technologies in a way that makes them widely accessible for innovative
research. [!ac](MOOSE) is equally capable of solving small models on common laptops and the very
biggest FEM models ever attempted---all without any major changes to configuration or source
code. Since its inception, the [!ac](MOOSE) project has focused on both developer and computational
efficiency. Improved developer efficiency is achieved by leveraging existing algorithms and
technologies from several leading open-source packages. Additionally, [!ac](MOOSE) uses several
complementary parallel technologies (both the distributed-memory message passing paradigm and
shared-memory thread-based approaches are used) to lay an efficient computational foundation for
development. Using existing open technologies in this manner helps the developers reduce the scope of
the project and keeps the size of the [!ac](MOOSE) code base maintainable. This approach provides
users with state-of-the-art finite element and solver technology as a basis for the advanced coupling
and solution strategies mentioned previously.

[!ac](MOOSE)'s developers work openly with other package developers to make sure that cutting-edge
technologies are available through [!ac](MOOSE), providing researchers with competitive research
opportunities. [!ac](MOOSE) maintains a set of objects that hide parallel interfaces while exposing
advanced spatial and temporal coupling algorithms in the framework.  This accessible approach places
developmental technology into the hands of scientists and engineers, which can speed the pace of
scientific discovery.
