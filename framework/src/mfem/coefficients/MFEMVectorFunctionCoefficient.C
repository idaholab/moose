#include "MFEMVectorFunctionCoefficient.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionCoefficient);

InputParameters
MFEMVectorFunctionCoefficient::validParams()
{
  InputParameters params = MFEMVectorCoefficient::validParams();
  params.addParam<FunctionName>("function", 0, "The function to associated with the Dirichlet BC");
  return params;
}

MFEMVectorFunctionCoefficient::MFEMVectorFunctionCoefficient(const InputParameters & parameters)
  : MFEMVectorCoefficient(parameters),
    _func(getFunction("function")),
    _vector_coefficient(std::make_shared<mfem::VectorFunctionCoefficient>(
        3,
        [&](const mfem::Vector & p, double t, mfem::Vector & u)
        {
          libMesh::RealVectorValue vector_value = _func.vectorValue(t, PointFromMFEMVector(p));
          u[0] = vector_value(0);
          u[1] = vector_value(1);
          u[2] = vector_value(2);
        }))
{
}

MFEMVectorFunctionCoefficient::~MFEMVectorFunctionCoefficient() {}
