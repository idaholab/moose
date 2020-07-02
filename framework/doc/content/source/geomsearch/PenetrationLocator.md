# PenetrationLocator

- A PenetrationLocator provides the perpendicular distance from a Secondary node to a Primary side and the "contact point" on the Primary side.
- The distance returned is negative if penetration hasn't yet occurred and positive if it has.
- To get a NearestNodeLocator `#include "PenetrationLocator.h"` and call `getPenetrationLocator(primary_id, secondary_id)` to create the object.
- The algorithm in PenetrationLocator utilizes a [`NearestNodeLocator`](/NearestNodeLocator.md) so `patch_size` is still important.

!media media/geomsearch/penetration_diagram.jpg
