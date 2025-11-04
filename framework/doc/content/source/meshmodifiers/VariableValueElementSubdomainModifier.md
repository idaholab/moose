# VariableValueElementSubdomainModifier

The `VariableValueElementSubdomainModifier` MeshModifier adjusts the element's subdomain according to a provided variable value. Subdomain IDs can be assigned based on the average variable value within the element, rounded to the nearest existing subdomain ID in the mesh.

**Note:** If the target value is not found in the subdomain ID list, a warning will be triggered. The system will then assign the smallest subdomain ID in the mesh that matches or exceeds the target subdomain ID to the element. Alternatively, if all subdomain IDs are smaller than the target, the system will assign the largest subdomain ID available in the mesh to the element.


!syntax parameters /MeshModifiers/VariableValueElementSubdomainModifier

!syntax inputs /MeshModifiers/VariableValueElementSubdomainModifier

!syntax children /MeshModifiers/VariableValueElementSubdomainModifier
