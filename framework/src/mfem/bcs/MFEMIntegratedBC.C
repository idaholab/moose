#ifdef MFEM_ENABLED

#include "MFEMIntegratedBC.h"

InputParameters
MFEMIntegratedBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  return params;
}

MFEMIntegratedBC::MFEMIntegratedBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters)
{
}

#endif
