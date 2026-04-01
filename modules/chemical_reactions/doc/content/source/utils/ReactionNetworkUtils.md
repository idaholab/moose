# ReactionNetworkUtils

!alert note
For the `[ReactionNetwork]` syntax parser, see [this page](chemical_reactions/index.md) instead.
This utility is currently used by the `[Physics]` syntax.

## Reaction network parser using a Parsed Expression Grammar

The chemical reactions module includes a reaction network parser this utility that enables
chemical reactions to be specified in a natural form in the input file.
This parser is notably used by the [AqueousReactionsEquilibriumPhysics.md].

The input file syntax for reactions has to be written in the following form:

\begin{equation}
\begin{aligned}
\nu_{11} \mathcal{A}_1 + \nu_{21} \mathcal{A}_2 + \ldots \text{->}& \mathcal{B}_{1} + \nu_{31} C_{2} \, [\log10\_K=2]\\
\nu_{12} \mathcal{A}_1 + \nu_{22} \mathcal{A}_2 + \ldots \text{->}& \mathcal{A}_{eq2} \, [K=100]
\end{aligned}
\end{equation}

Individual reactions are provided with the reactant species on the left hand side, while
the product species follow the `->` sign, followed by metadata in between square brackets. A
linebreak is used to delimit reactions, so that multiple reactions can be entered.

!alert note
The [AqueousReactionsEquilibriumPhysics.md] currently only supports one product species per reaction, on the right hand side,
as shown in the second equation in the example.

$\nu_{ij}$ are coefficients which have to be floating point number in the regular non-scientific notation.
Metadata keys and values are parsed as strings. The values may be floating point numbers or names of properties,
though [AqueousReactionsEquilibriumPhysics.md] only supports numbers at this time.
The parsing of the metadata is implemented in the classes using this utility.

## Troubleshooting

If the reaction network parsing fails, it will error with:

!listing utils/ReactionNetworkUtils.C start=!parser.parse(reaction_network_string, reactions) end=mooseError include-start=false include-end=true

In that case, you may examine the expected syntax below. The `grammar` is used by the parser to understand the input.
`*`, `+`, `?` and other symbols use the same rules as for regular expression parsing.

!listing utils/ReactionNetworkUtils.C start=parser end=mooseAssert
