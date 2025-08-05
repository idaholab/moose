//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSumAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSumAux);

/*
Class to set an H(curl) auxvariable to be the gradient of a H1 scalar variable.
*/
InputParameters
MFEMSumAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the gradient of an H1 conforming source variable and stores the result"
      " on an H(curl) conforming ND result auxvariable");
  params.addRequiredParam<VariableName>("source1",
                                        "Scalar H1 MFEMVariable to take the gradient of.");
  params.addRequiredParam<VariableName>("source2",
                                          "Scalar H1 MFEMVariable to take the gradient of.");                                        
  params.addParam<mfem::real_t>("scale_factor", 1.0, "Factor to scale result auxvariable by.");
  return params;
}

MFEMSumAux::MFEMSumAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source1_var_name(getParam<VariableName>("source1")),
    _source2_var_name(getParam<VariableName>("source2")),
    _source1_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source1_var_name)),
    _source2_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source2_var_name)),    
    _scale_factor(getParam<mfem::real_t>("scale_factor"))
{
}

// Computes the auxvariable.
void
MFEMSumAux::execute()
{
  add(_source1_var, _scale_factor, _source2_var, _result_var);
}

#endif
