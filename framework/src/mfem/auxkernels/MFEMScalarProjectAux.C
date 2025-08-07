#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarProjectAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarProjectAux);

InputParameters
MFEMScalarProjectAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects a Scalar Coefficient into a scalea MFEMVariable");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient",
                                                     "Name of the Scalar Coefficient to project.");
  return params;
}

MFEMScalarProjectAux::MFEMScalarProjectAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

void
MFEMScalarProjectAux::execute()
{
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_coef);
}

#endif
