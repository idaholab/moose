#include "MFEMSolverBase.h"

namespace platypus
{

InputParameters
MFEMSolverBase::validParams()
{
  return MFEMGeneralUserObject::validParams();
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}

} // platypus
