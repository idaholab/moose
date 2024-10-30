#include "MFEMConstantCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMConstantCoefficient);

InputParameters
MFEMConstantCoefficient::validParams()
{
  InputParameters params = MFEMCoefficient::validParams();
  params.addClassDescription(
      "Class for defining an mfem::ConstantCoefficient object to add to an MFEMProblem.");
  params.addRequiredParam<double>("value", "Value for the ConstantCoefficient");
  return params;
}

MFEMConstantCoefficient::MFEMConstantCoefficient(const InputParameters & parameters)
  : MFEMCoefficient(parameters),
    coefficient{getMFEMProblem().makeScalarCoefficient<mfem::ConstantCoefficient>(
        getParam<double>("value"))}
{
}

MFEMConstantCoefficient::~MFEMConstantCoefficient() {}
