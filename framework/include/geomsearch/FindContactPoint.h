
// Moose
#include "Moose.h"
#include "PenetrationLocator.h"

// libMesh
#include "elem.h"
#include "vector_value.h"

namespace libMesh
{
  class FEBase;
  class FEType;
}

namespace Moose
{

void
findContactPoint(PenetrationLocator::PenetrationInfo & p_info,
                 FEBase * _fe, FEType & _fe_type, const Point & slave_point,
                 bool start_with_centroid, const Real tangential_tolerance,
                 bool & contact_point_on_side);

void
restrictPointToFace(Point& p,
                    const Elem* side,
                    std::vector<Node*> &off_edge_nodes);

}

