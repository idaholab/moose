# SubdomainExtraElementIDGenerator

!syntax description /Mesh/SubdomainExtraElementIDGenerator

## Overview

The `SubdomainExtraElementIDGenerator` assigns extra element IDs based on mesh subdomain IDs of an input mesh.
If the extra element IDs do not exist in the input mesh, this mesh generator will add them.
The [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter can accept both subdomain names and subdomain IDs.
The [!param](/Mesh/SubdomainExtraElementIDGenerator/extra_element_ids) parameter is a two-dimensional vector separated by semicolon and then white space as normal MOOSE input syntax for two-dimensional vectors.
Its first dimensitionality must be equal to the size of the [!param](/Mesh/SubdomainExtraElementIDGenerator/extra_element_id_names) parameter and the size of each sub-vector must be equal to the size of the [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter.

!listing subdomain_elem_ids_test.i block=Mesh id=list:reactor_subdomain_extra_id_mesh caption=A sample input for the extra_element_ids parameter

In the above sample input, we have four subdomains listed in the [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter and three extra element id names in the [!param](/Mesh/SubdomainExtraElementIDGenerator/extra_element_id_names) parameter, thus we have two semicolons in [!param](/Mesh/SubdomainExtraElementIDGenerator/extra_element_ids) to separate the numbers into three groups for the three id names accordingly and each group has four numbers corresponding to the four subdomains.

Default element IDs can be provided through [!param](/Mesh/SubdomainExtraElementIDGenerator/default_extra_element_ids) for elements in subdomains that are not specified in the [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter.
Because of this, [!param](/Mesh/SubdomainExtraElementIDGenerator/default_extra_element_ids) is active only when the [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) parameter does not include all mesh subdomains.

!alert note
If [!param](/Mesh/SubdomainExtraElementIDGenerator/default_extra_element_ids) is not set and [!param](/Mesh/SubdomainExtraElementIDGenerator/subdomains) does not contain all mesh subdomains, when the extra element IDs already existed in the mesh,
the extra IDs for those elements of the missing subdomains will not be changed; or when the extra element IDs are newly added by this mesh generator, invalid IDs are set to elements of those missing subdomains.

!syntax parameters /Mesh/SubdomainExtraElementIDGenerator

!syntax inputs /Mesh/SubdomainExtraElementIDGenerator

!syntax children /Mesh/SubdomainExtraElementIDGenerator
