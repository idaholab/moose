# ExtraElementIDCopyGenerator

!syntax description /Mesh/ExtraElementIDCopyGenerator

## Overview

The `ExtraElementIDCopyGenerator` is used to copy an extra element ID to other extra element IDs.
If the target extra element IDs do not exist in the input mesh, they will be added by this mesh generator before copying.
The extra element ID name *subdomain_id* is reserved for the subdomain ID and can be used as the name of the source extra element ID, but not as the target extra element ID.
The extra element ID name *element_id* is reserved for the element ID and can be used as the name of the source extra element ID, but not as the target extra element ID.

!syntax parameters /Mesh/ExtraElementIDCopyGenerator

!syntax inputs /Mesh/ExtraElementIDCopyGenerator

!syntax children /Mesh/ExtraElementIDCopyGenerator
