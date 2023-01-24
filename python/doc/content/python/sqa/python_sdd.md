!template load file=sqa/sdd.md.template project=MOOSE Tools

!template! item key=system-purpose
!include python/sqa/python_system_purpose.md
!template-end!

!template! item key=system-scope
!include python/sqa/python_system_scope.md
!template-end!

!template! item key=dependencies-and-limitations
[!ac](MOOSE) Tools has several dependencies on other software packages and has scope that
is constantly evolving based upon funding, resources, priorities, and lab direction as the [!ac](MOOSE)
framework expands. However, the software is open-source and many features and even bugs can be offloaded
to developers with appropriate levels of knowledge and direction from the main design team. The primary
list of software dependencies is listed in the [python/sqa/python_sll.md]. This list is not meant to
be exhaustive. Individual operating systems may require specific packages to be installed prior to
using [!ac](MOOSE) Tools, which can be found on the [Install MOOSE](getting_started/installation/index.md optional=True) pages.
!template-end!
