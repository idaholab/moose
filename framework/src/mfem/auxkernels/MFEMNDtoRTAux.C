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
    params.addClassDescription( "Copies the DoF coefficints of a 2D Nedelec H(curl) MFEM Variable"
        "into a Raviart-Thomas H(div) MFEM Variable. In 2D ONLY this represents a 90 degree rotation"
        "because the RT basis is the rotated ND basis.");
    params.addRequiredParam<VariableName>("nd_source",
                                            "Name of H(curl) conforming ND variable to copy.");
    params.addParam<mfem::real_t>("sign", 1.0, "Optional sign multiplier.");
    return params;
}

MFEMNDtoRTAux::MFEMNDtoRTAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _nd_source_var_name(getParam<VariableName>("nd_source")),
    _nd_source_var(*getMFEMProblem().getGridFunction(_nd_source_var_name)),
    _sign(getParam<mfem::real_t>("sign"))
{
    const mfem::ParFiniteElementSpace * source_fes = _nd_source_var.ParFESpace();
    const mfem::ParFiniteElementSpace * target_fes = _result_var.ParFESpace();

    if (!source_fes)
        paramError("nd_source", "The source ND variable has no valid ParFiniteElementSpace.");
    
    if (!target_fes)
        mooseError("The target RT variable has no valid ParFiniteElementSpace.");

    if (source_fes->GetMesh()->Dimension() != 2 || target_fes->GetMesh()->Dimension() != 2)
        mooseError("MFEMNDtoRTAux is only valid in 2D.");

    if (_nd_source_var.Size() != _result_var.Size())
    paramError("nd_source",
                "The source ND variable and target RT variable must have the same local DoF size. "
                "Source size = ",
                _nd_source_var.Size(),
                ", target size = ",
                _result_var.Size(),
                ".");

    if (_sign != 1.0 && _sign !=-1.0)
        paramError("sign","The sign parameter must be either 1.0 or -1.0.");

}

void
MFEMNDtoRTAux::update()
{
    _result_var = _nd_source_var;

    if (_sign == -1.0)
        _result_var *= -1.0;
}

#endif
