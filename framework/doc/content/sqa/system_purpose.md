[!ac](MOOSE) is a tool for solving complex coupled
Multiphysics equations using the finite element method. [!ac](MOOSE) uses an object-oriented design
to abstract data structure management, parallelism, threading and compiling while providing an easy
to use interface targeted at engineers that may not have a lot of software development
experience. [!ac](MOOSE)  will require extreme scalability and flexibility when compared to other
FEM frameworks. For instance, [!ac](MOOSE) needs the ability to run extremely complex material
models, or even third-party applications within a parallel simulation without sacrificing
parallelism. This capability is in contrast to what is often seen in commercial packages, where
custom material models can limit the parallel scalability, forcing serial runs in the most severe
cases. When comparing high-end capabilities, many [!ac](MOOSE) competitors target modest-sized
clusters with just a few thousand processing cores. [!ac](MOOSE), however, will be required to
routinely executed on much larger clusters with scalability to clusters available in the top 500
systems ([top500.org](http://www.top500.org)). [!ac](MOOSE) will also be targeted at smaller systems
such as high-end laptop computers.

The design goal of [!ac](MOOSE) is to give developers ultimate control over their physical models
and applications. Designing new models or solving completely new classes of problems will be
accomplished by writing standard C++ source code within the framework's class hierarchy. Scientists
and engineers will be free to implement completely new algorithms using pieces of the framework where
possible, and extending the framework's capabilities where it makes sense to do so. Commercial
applications do not have this capability, and instead opt for either a more rigid parameter system or
a limited application-specific metalanguage.
