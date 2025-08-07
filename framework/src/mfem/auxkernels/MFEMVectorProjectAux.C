#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorProjectAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorProjectAux);

InputParameters
MFEMVectorProjectAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects a Vector Coefficient into a vector MFEMVariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>("coefficient",
                                                     "Name of the Vector Coefficient to project.");
  return params;
}

MFEMVectorProjectAux::MFEMVectorProjectAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _vec_coef(getVectorCoefficient("coefficient"))

{
}

void
MFEMVectorProjectAux::execute()
{

  _result_var = 0.0;
  _result_var.ProjectCoefficient(_vec_coef);
}

#endif
