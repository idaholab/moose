#include "MFEMVectorConstantCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMVectorConstantCoefficient);

InputParameters
MFEMVectorConstantCoefficient::validParams()
{
  InputParameters params = MFEMVectorCoefficient::validParams();
  params.addClassDescription(
      "Class for defining an mfem::VectorConstantCoefficient object to add to an MFEMProblem.");
  params.addRequiredParam<double>("value_x", "x component of the VectorConstantCoefficient");
  params.addRequiredParam<double>("value_y", "y component of the VectorConstantCoefficient");
  params.addRequiredParam<double>("value_z", "z component of the VectorConstantCoefficient");
  return params;
}

MFEMVectorConstantCoefficient::MFEMVectorConstantCoefficient(const InputParameters & parameters)
  : MFEMVectorCoefficient(parameters),
    _vector(
        {getParam<double>("value_x"), getParam<double>("value_y"), getParam<double>("value_z")}),
    _vector_coefficient{
        getMFEMProblem().makeVectorCoefficient<mfem::VectorConstantCoefficient>(_vector)}
{
}

MFEMVectorConstantCoefficient::~MFEMVectorConstantCoefficient() {}
