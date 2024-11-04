#include "MFEMVectorDirichletBCBase.h"

InputParameters
MFEMVectorDirichletBCBase::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "vector_coefficient", "The vector MFEM coefficient to use in the Dirichlet condition");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorDirichletBCBase::MFEMVectorDirichletBCBase(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_value(getParam<std::vector<Real>>("values")),
    _vec_coef(
        getMFEMProblem().getProblemData()._vector_manager.make<mfem::VectorConstantCoefficient>(
            mfem::Vector(_vec_value.data(), _vec_value.size())))
{
}
