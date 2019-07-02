!template load file=sqa/app_srs.md.template app=MOOSE category=_empty_

!template item key=introduction
MOOSE itself is comprised of the framework and a set of physics modules. As such, the [!ac](SRS)
for MOOSE is comprised of content from the framework as well as the physics modules, with each
portion adhering to an [!ac](SRS) document, as linked in [#dependencies] section.

!template item key=requirements
All of the requirements for [!ac](MOOSE) are provided in the dependent [!ac](SRS) documents, please
refer to the documents listed in [#dependencies].
