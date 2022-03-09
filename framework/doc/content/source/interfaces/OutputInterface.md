# OutputInterface

Interface to handle the restriction of output from objects to certain [Outputs](syntax/Outputs/index.md).
Numerous objects inherit this interface to output some of their
attributes and related quantities. An important example are
[Materials](syntax/Materials/index.md) which can output
material properties to a selection of outputs using an `outputs` parameter.
