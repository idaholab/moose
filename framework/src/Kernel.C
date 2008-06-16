#include "Kernel.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"

void
Kernel::computeResidual(DenseVector<Number> & Re, Elem * elem)
{
  _dof_map.dof_indices(elem, _dof_indices);
  _fe->reinit(elem); 
  Re.resize(_dof_indices.size());
}
