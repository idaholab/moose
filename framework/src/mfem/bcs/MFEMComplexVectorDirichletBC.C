#include "MFEMComplexVectorDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMComplexVectorDirichletBC);

InputParameters
MFEMComplexVectorDirichletBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  params.addRequiredParam<UserObjectName>(
      "real_vector_coefficient",
      "The vector MFEM coefficient representing the real component of the variable on which the "
      "Dirichlet condition is being applied");
  params.addRequiredParam<UserObjectName>("imag_vector_coefficient",
                                          "The vector MFEM coefficient representing the imaginary "
                                          "component of the variable on which the "
                                          "Dirichlet condition is being applied");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMComplexVectorDirichletBC::MFEMComplexVectorDirichletBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters),
    _vec_coef_re(const_cast<MFEMVectorCoefficient *>(
        &getUserObject<MFEMVectorCoefficient>("real_vector_coefficient"))),
    _vec_coef_im(const_cast<MFEMVectorCoefficient *>(
        &getUserObject<MFEMVectorCoefficient>("imag_vector_coefficient")))
{
  _boundary_condition =
      std::make_shared<hephaestus::VectorDirichletBC>(getParam<std::string>("variable"),
                                                      bdr_attr,
                                                      _vec_coef_re->getVectorCoefficient().get(),
                                                      _vec_coef_im->getVectorCoefficient().get());
}
