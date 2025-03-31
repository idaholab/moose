#ifdef MFEM_ENABLED

#include "MFEMGeneralUserObject.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGeneralUserObject);

InputParameters
MFEMGeneralUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for custom GeneralUserObjects to add to MFEM problems.");
  return params;
}

MFEMGeneralUserObject::MFEMGeneralUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _mfem_problem(static_cast<MFEMProblem &>(_fe_problem))
{
}

mfem::Coefficient &
MFEMGeneralUserObject::getScalarCoefficient(const std::string & name)
{
  return getMFEMProblem().getProperties().getScalarCoefficient(name);
}

mfem::VectorCoefficient &
MFEMGeneralUserObject::getVectorCoefficient(const std::string & name)
{
  return getMFEMProblem().getProperties().getVectorCoefficient(name);
}

mfem::MatrixCoefficient &
MFEMGeneralUserObject::getMatrixCoefficient(const std::string & name)
{
  return getMFEMProblem().getProperties().getMatrixCoefficient(name);
}
