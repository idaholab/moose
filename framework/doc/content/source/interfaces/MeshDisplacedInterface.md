# MeshDisplacedInterface

This interface is designed to let MOOSE objects performs tasks when the displaced mesh is updated.
When the displaced mesh gets updated, MOOSE will call the `meshDisplaced` function of all `MooseObjects` derived from this interface.
Developers can put code in this function for updating the object according the changes in the displaced mesh.
It is noted that this function is called only when MOOSE detects a change in the displaced mesh (and skipped if the displacements are not modified).
