//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMParaViewDataCollection.h"

registerMooseObject("MooseApp", MFEMParaViewDataCollection);

InputParameters
MFEMParaViewDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling export to an mfem::ParaViewDataCollection.");
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  params.addParam<bool>("high_order_output",
                        false,
                        "Sets whether or not to output the data as "
                        "high-order elements (false by default)."
                        "Reading high-order data requires ParaView"
                        "5.5 or later.");
  params.addParam<std::vector<MFEMScalarCoefficientName>>(
      "scalar_coefficients",
      {},
      "Optional set of scalar coefficient names to evaluate and include in output.");
  params.addParam<std::vector<MFEMVectorCoefficientName>>(
      "vector_coefficients",
      {},
      "Optional set of vector coefficient names to evaluate and include in output.");

  MooseEnum vtk_format("ASCII BINARY BINARY32", "BINARY", true);
  params.addParam<MooseEnum>(
      "vtk_format",
      vtk_format,
      "Select VTK data format to use, choosing between BINARY, BINARY32, and ASCII.");
  return params;
}

MFEMParaViewDataCollection::MFEMParaViewDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters),
    _pv_dc((_file_base + std::string("/Run") + std::to_string(getFileNumber())).c_str(), &_pmesh),
    _scalar_coefficient_names(
        getParam<std::vector<MFEMScalarCoefficientName>>("scalar_coefficients")),
    _vector_coefficient_names(
        getParam<std::vector<MFEMVectorCoefficientName>>("vector_coefficients")),
    _high_order_output(getParam<bool>("high_order_output")),
    _refinements(getParam<unsigned int>("refinements")),
    _vtk_format(parameters.get<MooseEnum>("vtk_format").getEnum<mfem::VTKFormat>())
{
  _pv_dc.SetPrecision(9);
  _pv_dc.SetHighOrderOutput(_high_order_output);
  _pv_dc.SetLevelsOfDetail(_refinements + 1);
  _pv_dc.SetDataFormat(_vtk_format);
  registerFields();
  registerScalarCoefficients(_scalar_coefficient_names);
  registerVectorCoefficients(_vector_coefficient_names);
}

void
MFEMParaViewDataCollection::registerScalarCoefficients(
    std::vector<MFEMScalarCoefficientName> & scalar_coefficient_names)
{
  for (const auto & scalar_coefficient_name : scalar_coefficient_names)
  {
    std::string coef_name(scalar_coefficient_name + "_coef");
    _pv_dc.RegisterCoeffField(
        coef_name, &_problem_data.coefficients.getScalarCoefficient(scalar_coefficient_name));
  }
}

void
MFEMParaViewDataCollection::registerVectorCoefficients(
    std::vector<MFEMVectorCoefficientName> & vector_coefficient_names)
{
  for (const auto & vector_coefficient_name : vector_coefficient_names)
  {
    std::string vec_coef_name(vector_coefficient_name + "_coef");
    _pv_dc.RegisterVCoeffField(
        vec_coef_name, &_problem_data.coefficients.getVectorCoefficient(vector_coefficient_name));
  }
}

#endif
