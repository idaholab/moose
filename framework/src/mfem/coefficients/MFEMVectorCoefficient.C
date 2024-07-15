#include "MFEMVectorCoefficient.h"

registerMooseObject("PlatypusApp", MFEMVectorCoefficient);

InputParameters
MFEMVectorCoefficient::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMVectorCoefficient");
  return params;
}

MFEMVectorCoefficient::MFEMVectorCoefficient(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}

MFEMVectorCoefficient::~MFEMVectorCoefficient() {}
