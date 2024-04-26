# ModifyElementSubdomainByVar

The `ModifyElementSubdomainByVar` UserObject adjusts the element's subdomain according to a provided variable value. Subdomain IDs can be assigned based on the average variable value within the element, rounded to the nearest existing subdomain ID in the mesh.

**Note:** If the desired value is not present in the subdomain ID list, a warning will be issued, and the closest existing subdomain ID value will be automatically assigned to the element.


!syntax parameters /UserObjects/ModifyElementSubdomainByVar

!syntax inputs /UserObjects/ModifyElementSubdomainByVar

!syntax children /UserObjects/ModifyElementSubdomainByVar
