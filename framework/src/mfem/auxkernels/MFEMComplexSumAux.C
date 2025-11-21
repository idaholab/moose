//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexSumAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexSumAux);

InputParameters
MFEMComplexSumAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the sum of an arbitrary number of complex variables sharing an FE space, each "
      "optionally scaled by a complex constant, and stores the result in an auxiliary complex variable.");
  params.addRequiredParam<std::vector<VariableName>>("source_variables",
                                                     "The names of the MFEM complex variables to sum");
  params.addParam<std::vector<mfem::real_t>>(
      "scale_factors", "The factors to scale each MFEM variable by during summation");
  return params;
}

MFEMComplexSumAux::MFEMComplexSumAux(const InputParameters & parameters)
  : MFEMComplexAuxKernel(parameters),
    _var_names(getParam<std::vector<VariableName>>("source_variables")),
    _scale_factors_real(parameters.isParamValid("scale_factors_real")
                       ? getParam<std::vector<mfem::real_t>>("scale_factors_real")
                       : std::vector<mfem::real_t>(_var_names.size(), 1.0)),
    _scale_factors_imag(parameters.isParamValid("scale_factors_imag")
                       ? getParam<std::vector<mfem::real_t>>("scale_factors_imag")
                       : std::vector<mfem::real_t>(_var_names.size(), 1.0))
{
  if (_var_names.size() != _scale_factors_real.size())
    paramError("scale_factors_real",
               "Number of MFEM variables to sum over is different from the number of provided "
               "real scale factors.");
  if (_var_names.size() != _scale_factors_imag.size())
    paramError("scale_factors_imag",
               "Number of MFEM variables to sum over is different from the number of provided "
               "imaginary scale factors.");

  for (const auto & var_name : _var_names)
  {
    const mfem::ParComplexGridFunction * gf =
        getMFEMProblem().getProblemData().cmplx_gridfunctions.Get(var_name);
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
MFEMComplexSumAux::execute()
{
  // result = sum_i ((_scale_factor_i_real+i*scale_factor_i_imag) * _summed_var_i)
  _result_var.real() = 0.0;
  _result_var.imag() = 0.0;
  for (const auto i : index_range(_summed_vars))
  {
    _result_var.real().Add(_scale_factors_real[i], _summed_vars[i]->real());
    _result_var.real().Add(-_scale_factors_imag[i], _summed_vars[i]->imag());
    _result_var.imag().Add(_scale_factors_real[i], _summed_vars[i]->imag());
    _result_var.imag().Add(_scale_factors_imag[i], _summed_vars[i]->real());
  }
    

}

#endif
