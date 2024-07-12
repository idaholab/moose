#include "MFEMSolverBase.h"

InputParameters
MFEMSolverBase::validParams()
{
  return MFEMGeneralUserObject::validParams();
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
