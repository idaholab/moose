#include "MFEMPostprocessor.h"

InputParameters
MFEMPostprocessor::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

MFEMPostprocessor::MFEMPostprocessor(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), Postprocessor(this)
{
}
