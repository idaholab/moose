#include "MFEMFormulation.h"

registerMooseObject("PlatypusApp", MFEMFormulation);

InputParameters
MFEMFormulation::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFormulation");
  return params;
}

MFEMFormulation::MFEMFormulation(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
