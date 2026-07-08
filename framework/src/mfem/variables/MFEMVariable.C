//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVariable.h"
#include "MFEMProblem.h"
#include "MFEMFESpaceHierarchy.h"
#include "MooseVariableBase.h"
#include "MFEMVectorMagnitudeCoefficient.h"

registerMooseObject("MooseApp", MFEMVariable);

InputParameters
MFEMVariable::validParams()
{
  InputParameters params = MFEMObject::validParams();
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries.
  params.addParam<MFEMFESpaceName>("fespace",
                                   "The finite element space this variable is defined on.");
  params.addParam<std::string>(
      "fespace_hierarchy",
      "Name of a FESpaceHierarchy; the variable lives on its finest level. "
      "Mutually exclusive with 'fespace'.");
  // Require moose variable parameters (not used!)
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding MFEM variables to the problem (`mfem::ParGridFunction`s).");
  params.registerBase("MooseVariableBase");
  params.registerSystemAttributeName("MooseVariableBase");
  params.addParam<VariableName>(
      "time_derivative",
      "Optional name to assign to the time derivative of the variable in transient problems.");
  return params;
}

MFEMVariable::MFEMVariable(const InputParameters & parameters)
  : MFEMObject(parameters),
    _time_derivative_name(
        isParamValid("time_derivative")
            ? getParam<VariableName>("time_derivative")
            : VariableName(
                  getMFEMProblem().getProblemData().time_derivative_map.createTimeDerivativeName(
                      name())))
{
  const bool has_fespace = isParamSetByUser("fespace");
  const bool has_hierarchy = isParamSetByUser("fespace_hierarchy");

  if (has_fespace && has_hierarchy)
    paramError("fespace_hierarchy", "Cannot specify both 'fespace' and 'fespace_hierarchy'.");
  if (!has_fespace && !has_hierarchy)
    paramError("fespace", "Either 'fespace' or 'fespace_hierarchy' must be provided.");

  if (has_fespace)
  {
    _fespace_ptr = &getMFEMProblem().getMFEMObject<MFEMFESpace>(
        "MFEMFESpace", getParam<MFEMFESpaceName>("fespace"));
    _par_fespace = _fespace_ptr->getFESpace();
    _is_scalar = _fespace_ptr->isScalar();
  }
  else
  {
    const auto & hierarchy_name = getParam<std::string>("fespace_hierarchy");
    const auto & hierarchy = getMFEMProblem().getMFEMObject<MFEMFESpaceHierarchy>(
        "MFEMFESpaceHierarchy", hierarchy_name);
    _par_fespace = getMFEMProblem().getProblemData().fespaces.GetShared(hierarchy_name);
    _is_scalar = hierarchy.isScalar();
  }

  _gridfunction = buildGridFunction();
  *_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParGridFunction>
MFEMVariable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_par_fespace.get());
}

void
MFEMVariable::declareCoefficients()
{
  const int cont_type = _par_fespace->FEColl()->GetContType();
  if (_is_scalar)
  {
    getMFEMProblem().getCoefficients().declareScalar<mfem::GridFunctionCoefficient>(
        name(), getGridFunction().get());
    // If gradient is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::CONTINUOUS)
    {
      getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
          name() + "_grad", getGridFunction().get());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_grad_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_grad"));
    }
  }
  else
  {
    getMFEMProblem().getCoefficients().declareVector<mfem::VectorGridFunctionCoefficient>(
        name(), getGridFunction().get());
    getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
        name() + "_mag", getMFEMProblem().getCoefficients().getVectorCoefficient(name()));
    // If curl is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::TANGENTIAL ||
        cont_type == mfem::FiniteElementCollection::CONTINUOUS)
    {
      getMFEMProblem().getCoefficients().declareVector<mfem::CurlGridFunctionCoefficient>(
          name() + "_curl", getGridFunction().get());
      getMFEMProblem().getCoefficients().declareScalar<MFEMVectorMagnitudeCoefficient>(
          name() + "_curl_mag",
          getMFEMProblem().getCoefficients().getVectorCoefficient(name() + "_curl"));
    }
    // If divergence is well-defined on this variable, create auxiliary coefficient
    if (cont_type == mfem::FiniteElementCollection::NORMAL ||
        cont_type == mfem::FiniteElementCollection::CONTINUOUS)
      getMFEMProblem().getCoefficients().declareScalar<mfem::DivergenceGridFunctionCoefficient>(
          name() + "_div", getGridFunction().get());
  }
}

#endif
