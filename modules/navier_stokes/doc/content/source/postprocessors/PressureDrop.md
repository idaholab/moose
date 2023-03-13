# PressureDrop

!syntax description /Postprocessors/PressureDrop

The convention used is that if the pressure is decreasing from the upstream to downstream boundaries,
then the pressure drop is positive.

The weighting functor, a vector, is dotted with the normal of the boundary and can help compute more
representative pressure drops for the cases of non-constant
pressure profiles across a boundary or for handling multiple upstream/downstream boundaries.
This is effectively a weighting flux, which should usually be set to a momentum functor so
that the effective weighting factor is the local mass flux.

\begin{equation}
\begin{split}
P_{upstream} &= \dfrac{\int_{\delta \Omega}} P(x,y,z) \vec{W(x,y,z) \cdot \vec{n}}{\dfrac{\int_{\delta \Omega}} \vec{W(x,y,z) \cdot \vec{n}}} \\
P_{downstream} &= ... \\
P_{drop} &= P_{upstream} - P_{downstream}
\end{split}
\end{equation}

where $\delta \Omega$ is the boundary (upstream or downstream) of interest for each pressure evaluation,
$P$ is the local pressure (on the element side on the boundary for finite volume variables), $\vec{W}$ is
the vector weighting functor and $\vec{n}$ is the local element side normal.

!alert warning
The default weighting functor, a constant vector, will weigh more heavily surfaces that are not aligned
with the axis of the frame of reference! It is highly advised to specify a non-default weighting functor
for curved surfaces.

!alert note
If using mesh refinement, upstream or downstream boundaries may not be split between two refined elements,
children of the same parent element.

## Example input syntax

In this example, we measure the pressure drop between the inlet and outlet of a flow channel,
as well as between the inlet and a midsection.

!listing test/tests/postprocessors/pressure_drop/test.i block=Postprocessors

!syntax parameters /Postprocessors/PressureDrop

!syntax inputs /Postprocessors/PressureDrop

!syntax children /Postprocessors/PressureDrop
