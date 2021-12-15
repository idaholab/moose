//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialAuxBase.h"

defineLegacyParams(MaterialAuxBaseTempl<>);

// We need a specialization for outputting a RealEigenVector
// because we cannot construct _offset with "0", and we
// also need to do error checking to make sure the offset is
// the correct size.
template <>
MaterialAuxBaseTempl<RealEigenVector, false, RealEigenVector>::MaterialAuxBaseTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<RealEigenVector>(parameters),
    _prop(this->template getGenericMaterialProperty<RealEigenVector, false>("property")),
    _factor(this->template getParam<Real>("factor")),
    _offset(this->isParamValid("offset") ? this->template getParam<RealEigenVector>("offset")
                                         : RealEigenVector(this->_var.count()))
{
  if (_offset.size() != this->_var.count())
    this->paramError("offset",
                     "Has ",
                     _offset.size(),
                     " component(s) but the variable ",
                     this->template getParam<AuxVariableName>("variable"),
                     " has ",
                     this->_var.count(),
                     " component(s)");
}
