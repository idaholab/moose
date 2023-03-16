## Conda Alternatives

While we have no other means of providing 'pre-built' libraries, you are not limited to only Conda.
As long you meet our minimum requirements (listed below), you should be able to build your own
library stack capable of MOOSE based development.

[Homebrew](https://brew.sh/), [MacPorts](https://www.macports.org/),
[Fink](https://www.finkproject.org/) are all good alternatives for MacOS machines. These tools can
provide you with finished easy-to-install binaries, leaving you only needing to build PETSc, libMesh
and then MOOSE/your Application.

[Spack](https://spack.readthedocs.io/en/latest/index.html) is another alternative, capable of
building very optimized stacks for both Linux and MacOS. Spack however does not provide pre-built
binaries. Spack instead supplies 'recipes' intended to be run by you, to build the stack.

Perhaps our Advanced Instructions on our [Getting Started](getting_started/index.md) page would
suffice?

Lastly, there is always building everything from source if you're up to the challenge!

!include sqa/minimum_requirements.md
