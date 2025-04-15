#ifdef MFEM_ENABLED

#include "MFEMEssentialBC.h"

InputParameters
MFEMEssentialBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  return params;
}

MFEMEssentialBC::MFEMEssentialBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters)
{
}

#endif
