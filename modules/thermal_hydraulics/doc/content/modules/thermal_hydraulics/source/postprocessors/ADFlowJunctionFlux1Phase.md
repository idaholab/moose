# ADFlowJunctionFlux1Phase

This post-processor is used to query an entry in the flux vector for a 1-phase
flow junction. The user specifies the boundary of the flow channel connected
to the junction via `boundary`, along with the name of the junction component
with the `junction` parameter. The parameter `connection_index` corresponds to
the index of the given boundary within the `connections` parameter of the
junction component (starting with index 0). Lastly, the queried entry within
the flux vector is chosen via the `equation` parameter.

## Troubleshooting

Note that if you get an error message like the following:

```
*** ERROR ***
Unable to find user object with name 'my_junction:junction_uo'
```

then this means that the supplied `junction` parameter is not valid, either
because the component does not exist or is not a valid junction. In the latter
case, please contact the THM team for assistance.

!syntax parameters /Postprocessors/ADFlowJunctionFlux1Phase

!syntax inputs /Postprocessors/ADFlowJunctionFlux1Phase

!syntax children /Postprocessors/ADFlowJunctionFlux1Phase
