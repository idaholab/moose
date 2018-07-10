# PartitionerWeightTest

It is a test `partitioner` to demonstrate how to assign the element weights and the side weights to the graph.
The element weights (corresponding to work load) could be assigned by overriding `computeElementWeight`. The side
weights (related to communication) are added using `computeSideWeight`. In this particular example, we consider the
left half domain twice more expensive the right. The middle horizontal line is assumed to have ten times higher
communication.
