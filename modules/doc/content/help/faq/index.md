# Frequently Asked Questions

## Installation

- [I don't like Conda. Are there alternatives?](faq_conda_alternatives.md)
- [Conda commands are not working...](help/troubleshooting.md#condaissues)
- [I can't build my application or moose.](help/troubleshooting.md#buildissues)
- [I get a 'gethostbyname' failure.](help/troubleshooting.md#failingtests)
- [Some or all of my tests fail.](help/troubleshooting.md#failingtests)
- [How do I build my own libMesh?](faq/faq_build_libmesh.md)
- [How do I build my own libMesh with VTK?](faq/faq_build_libmesh-vtk.md)
- [How do I build my own PETSc?](faq/faq_build_petsc.md)

## Simulations

- [Where is the stiffness matrix?](help/faq/what_is_fem.md)
- [How do I execute MOOSE in parallel?](getting_started/examples_and_tutorials/tutorial01_app_development/step07_parallel.md optional=True)
- [Why is my solve not converging?](failed_solves.md)
- [It solves fine in serial/with threads, but as soon as I use MPI it does not converge](failed_solves.md#parallel)
- [What is happening behind the scenes in my simulation?](Debug/index.md)
  - [During setup](Debug/index.md#debug-setup)
  - [During execution](Debug/index.md#debug-order)
- Why is my solve/application slow?
  - [As a developer of the application](application_development/profiling.md)
  - [As a user](source/outputs/PerfGraphOutput.md)

## Civet Failures

- Troubleshoot errors only occuring on your PRs using our
  [Apptainer containers](help/inl/apptainer.md).

Numerous questions/answers can also be found in on the following resources.
Please search whether your question has already been answered before posting,
and always consult the [guidelines](https://github.com/idaholab/moose/discussions#)
before posting!

- [The MOOSE Discussion forum](https://github.com/idaholab/moose/discussions)
- [The old MOOSE Q&A mailing list (no new posts please)](https://groups.google.com/forum/#!forum/moose-users)
