#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBCBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMVectorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<std::vector<Real>>("values",
                                             "The vector which will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorDirichletBCBase::MFEMVectorDirichletBCBase(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_value(getParam<std::vector<Real>>("values")),
    _vec_coef(getMFEMProblem().makeVectorCoefficient<mfem::VectorConstantCoefficient>(
        mfem::Vector(_vec_value.data(), _vec_value.size())))
{
}

#endif
