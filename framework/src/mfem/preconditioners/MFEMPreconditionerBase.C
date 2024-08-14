#include "MFEMPreconditionerBase.h"

InputParameters
MFEMPreconditionerBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();

  params.registerBase("MFEMPreconditionerBase");

  return params;
}

MFEMPreconditionerBase::MFEMPreconditionerBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
