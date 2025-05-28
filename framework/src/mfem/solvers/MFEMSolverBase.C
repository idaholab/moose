#ifdef MFEM_ENABLED

#include "MFEMSolverBase.h"

InputParameters
MFEMSolverBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription("Base class for defining mfem::Solver derived classes for Moose.");
  params.registerBase("MFEMSolverBase");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");

  return params;
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), _lor{getParam<bool>("low_order_refined")}
{
}

#endif
