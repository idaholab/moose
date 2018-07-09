# PartitionerWeightTest

It is a test `partitioner` to demonstrate how to assign the vertex weights and the edge weights to the graph.
The vertex weights (corresponding to word load) could be assigned by overriding `computeElementWeight`. The edge
weights (related to communication) are added using `computeSideWeight`. In this particular example, we consider the
left half domain twice more expensive the right. The middle horizontal line is assumed to have ten times higher
communication.
