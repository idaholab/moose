!template load file=sqa/app_sdd.md.template app=MOOSE category=_empty_

!template item key=introduction
MOOSE itself is comprised of the framework and a set of physics modules. As such, the [!ac](SDD)
for MOOSE is comprised of content from the framework as well as the physics modules, with each
portion adhering to an [!ac](SDD) document, as linked in [#dependencies] section. These links
include a [!ac](SDD) that details the design of each portion of the software, when combined comprise
the complete set of design documents for {{app}}.

!template item key=dependencies-intro
The [!ac](SDD) for MOOSE as whole, including the framework, modules, and supporting infrastructure
is detailed in the files listed below.

!template item key=cross-reference-intro

!template item key=cross-reference
All of the design documents for [!ac](MOOSE) are provided in the dependent [!ac](SDD) documents,
please refer to the documents listed in [#dependencies].
