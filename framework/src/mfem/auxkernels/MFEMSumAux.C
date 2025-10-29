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

InputParameters
MFEMSumAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the sum of an arbitrary number of variables sharing an FE space, each "
      "optionally scaled by a real constant, and stores the result in an auxiliary variable.");
  params.addRequiredParam<std::vector<VariableName>>("source_variables",
                                                     "The names of the MFEM variables to sum");
  params.addParam<std::vector<mfem::real_t>>(
      "scale_factors", "The factors to scale each MFEM variable by during summation");
  return params;
}

MFEMSumAux::MFEMSumAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _var_names(getParam<std::vector<VariableName>>("source_variables")),
    _scale_factors(parameters.isParamValid("scale_factors")
                       ? getParam<std::vector<mfem::real_t>>("scale_factors")
                       : std::vector<mfem::real_t>(_var_names.size(), 1.0))
{
  if (_var_names.size() != _scale_factors.size())
    paramError("scale_factors",
               "Number of MFEM variables to sum over is different from the number of provided "
               "scale factors.");
  for (const auto & var_name : _var_names)
  {
    const mfem::ParGridFunction * gf =
        getMFEMProblem().getProblemData().gridfunctions.Get(var_name);
    if (gf->ParFESpace() == _result_var.ParFESpace())
      _summed_vars.push_back(gf);
    else
      paramError("source_variables",
                 "The MFEM variable ",
                 var_name,
                 " being summed has a different FESpace from ",
                 _result_var_name);
  }
}

void
MFEMSumAux::execute()
{
  // result = sum_i (_scale_factor_i * _summed_var_i)
  _result_var = 0.0;
  for (const auto i : index_range(_summed_vars))
    _result_var.Add(_scale_factors[i], *_summed_vars[i]);
}

#endif
