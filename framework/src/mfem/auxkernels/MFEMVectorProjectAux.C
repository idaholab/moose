#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorProjectAux.h"

registerMooseObject("MooseApp", MFEMVectorProjectAux);

InputParameters
MFEMVectorProjectAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription("Projects a vector coefficient onto a vector MFEMVariable.");
  params.addRequiredParam<MFEMVectorCoefficientName>("vector_coefficient",
                                                     "Name of the vector coefficient to project.");
  return params;
}

MFEMVectorProjectAux::MFEMVectorProjectAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters), _vec_coef(getVectorCoefficient("vector_coefficient"))
{
}

void
MFEMVectorProjectAux::execute()
{
  _result_var = 0.0;
  _result_var.ProjectCoefficient(_vec_coef);
}

#endif
