#include "MFEMVectorCoefficient.h"

registerMooseObject("PlatypusApp", MFEMVectorCoefficient);

InputParameters
MFEMVectorCoefficient::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription(
      "Base class for defining mfem::VectorCoefficient objects to add to an MFEMProblem.");
  params.registerBase("MFEMVectorCoefficient");
  return params;
}

MFEMVectorCoefficient::MFEMVectorCoefficient(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}

MFEMVectorCoefficient::~MFEMVectorCoefficient() {}
