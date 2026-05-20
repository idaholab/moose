//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNDtoRTAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMNDtoRTAux);

InputParameters
MFEMNDtoRTAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Copies the DoFs of a 2D Nedelec H(curl) MFEM Variable "
      "into a Raviart-Thomas H(div) MFEM Variable. In 2D ONLY this represents a 90 degree rotation "
      "because the RT basis is the rotated ND basis.");
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Name of H(curl) conforming ND variable to copy.");
  params.addParam<mfem::real_t>(
      "scale_factor",
      1.0,
      "Optional scale factor. Negative values can be used to flip the sign of the rotation.");

  return params;
}

MFEMNDtoRTAux::MFEMNDtoRTAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _nd_source_var_name(getParam<VariableName>("source")),
    _nd_source_var(*getMFEMProblem().getGridFunction(_nd_source_var_name)),
    _scale_factor(getParam<mfem::real_t>("scale_factor"))
{
  const mfem::ParFiniteElementSpace * source_fes = _nd_source_var.ParFESpace();
  const mfem::ParFiniteElementSpace * target_fes = _result_var.ParFESpace();

  if (!source_fes)
    paramError("source", "The source ND variable has no valid ParFiniteElementSpace.");

  if (!target_fes)
    mooseError("The target RT variable has no valid ParFiniteElementSpace.");

  const mfem::FiniteElementCollection * source_fec = source_fes->FEColl();
  const mfem::FiniteElementCollection * target_fec = target_fes->FEColl();

  if (!dynamic_cast<const mfem::ND_FECollection *>(source_fec))
    paramError("source",
               "The source variable must use an MFEM H(curl) Nedelec space. "
               "Detected FE collection: ",
               source_fec->Name(),
               ".");
  if (!dynamic_cast<const mfem::RT_FECollection *>(target_fec))
    mooseError("The target variable must use an MFEM H(div) Raviart-Thomas space. "
               "Detected FE collection: ",
               target_fec->Name(),
               ".");

  if (source_fes->GetMesh()->Dimension() != 2 || target_fes->GetMesh()->Dimension() != 2)
    mooseError("MFEMNDtoRTAux is only valid in 2D.");

  if (_nd_source_var.Size() != _result_var.Size())
    paramError("source",
               "The source ND variable and target RT variable must have the same local DoF size. "
               "Source size = ",
               _nd_source_var.Size(),
               ", target size = ",
               _result_var.Size(),
               ".");
}

void
MFEMNDtoRTAux::execute()
{
  _result_var = _nd_source_var;
  _result_var *= _scale_factor;
}

#endif
