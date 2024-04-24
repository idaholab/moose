#include "MFEMVectorCoefficient.h"

registerMooseObject("PlatypusApp", MFEMVectorCoefficient);

InputParameters
MFEMVectorCoefficient::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("MFEMVectorCoefficient");
  return params;
}

MFEMVectorCoefficient::MFEMVectorCoefficient(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

MFEMVectorCoefficient::~MFEMVectorCoefficient() {}
