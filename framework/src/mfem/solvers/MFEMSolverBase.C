#include "MFEMSolverBase.h"

registerMooseObject("PlatypusApp", MFEMSolverBase);

InputParameters
MFEMSolverBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();

  params.registerBase("MFEMSolverBase");

  return params;
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
