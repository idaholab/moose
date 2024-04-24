#include "MFEMConstantCoefficient.h"

registerMooseObject("PlatypusApp", MFEMConstantCoefficient);

InputParameters
MFEMConstantCoefficient::validParams()
{
  InputParameters params = MFEMCoefficient::validParams();
  params.addRequiredParam<double>("value", "Value for the ConstantCoefficient");
  return params;
}

MFEMConstantCoefficient::MFEMConstantCoefficient(const InputParameters & parameters)
  : MFEMCoefficient(parameters),
    coefficient{std::make_shared<mfem::ConstantCoefficient>(getParam<double>("value"))}
{
}

MFEMConstantCoefficient::~MFEMConstantCoefficient() {}
