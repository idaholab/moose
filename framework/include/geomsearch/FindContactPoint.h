
// Moose
#include "Moose.h"

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
findContactPoint(FEBase * _fe, FEType & _fe_type, const Elem * master_elem, unsigned int side_num, const Point & slave_point,
                 bool start_with_centroid, Point & contact_ref, Point & contact_phys, std::vector<std::vector<Real> > & side_phi,
                 Real & distance, RealGradient & normal, bool & contact_point_on_side);
}

