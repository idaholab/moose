# SubdomainExtraElementIDGenerator

!syntax description /Mesh/SubdomainExtraElementIDGenerator

## Overview

The `SubdomainExtraElementIDGenerator` assigns element IDs based on mesh subdomain IDs of an input mesh.
If the element IDs do not exist in the input mesh, they will be added by this mesh generator.
Default element IDs can be provided through [!param](/Mesh/SubdomainExtraElementIDGenerator/default_extra_element_ids) for elements in subdomains that are not specified in the [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter.

!alert note
If [!param](/Mesh/SubdomainExtraElementIDGenerator/default_extra_element_ids) is not set and [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) does not contain all mesh subdomains, when the extra element IDs already existed in the mesh,
the extra IDs for those elements of the missing subdomains will not be changed; or when the extra element IDs are newly added by this mesh generator, invalid IDs are set to elements of those missing subdomains.

!syntax parameters /Mesh/SubdomainExtraElementIDGenerator

!syntax inputs /Mesh/SubdomainExtraElementIDGenerator

!syntax children /Mesh/SubdomainExtraElementIDGenerator
