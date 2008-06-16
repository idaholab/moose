#include"Kernel.h"

void
Kernel::computeResidual(DenseVector<Number> & Re, Elem * elem)
{
  dof_map.dof_indices(elem, dof_indices);
  fe->reinit(elem);
  Re.resize(dof_indices.size());
}
