#include "MFEMGeneralUserObject.h"

registerMooseObject("PlatypusApp", MFEMGeneralUserObject);

InputParameters
MFEMGeneralUserObject::validParams()
{
  return GeneralUserObject::validParams();
}

MFEMGeneralUserObject::MFEMGeneralUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _mfem_problem(static_cast<MFEMProblem &>(_fe_problem))
{
}
