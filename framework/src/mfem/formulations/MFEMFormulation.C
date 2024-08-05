#include "MFEMFormulation.h"

registerMooseObject("PlatypusApp", MFEMFormulation);

InputParameters
MFEMFormulation::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFormulation");

  params.addClassDescription("Base class for addition of customised platypus::ProblemBuilders");
  return params;
}

MFEMFormulation::MFEMFormulation(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
