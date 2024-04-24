#include "MFEMFormulation.h"

registerMooseObject("PlatypusApp", MFEMFormulation);

InputParameters
MFEMFormulation::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("MFEMFormulation");
  return params;
}

MFEMFormulation::MFEMFormulation(const InputParameters & parameters) : GeneralUserObject(parameters)
{
}

MFEMFormulation::~MFEMFormulation() {}
