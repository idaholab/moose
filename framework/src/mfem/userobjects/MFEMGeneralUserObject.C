#include "MFEMGeneralUserObject.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMGeneralUserObject);

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
