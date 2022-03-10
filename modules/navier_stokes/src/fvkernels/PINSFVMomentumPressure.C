//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressure.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressure);

InputParameters
PINSFVMomentumPressure::validParams()
{
  InputParameters params = INSFVMomentumPressure::validParams();
  params.addClassDescription("Introduces the coupled pressure term $eps \nabla P$ into the "
                             "Navier-Stokes porous media momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");

  return params;
}

PINSFVMomentumPressure::PINSFVMomentumPressure(const InputParameters & params)
  : INSFVMomentumPressure(params), _eps(getFunctor<ADReal>(NS::porosity))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressure may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

void
PINSFVMomentumPressure::gatherRCData(const Elem & elem)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#else
  bool correct_skewness =
      (_p_var->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);
  const auto strong_residual =
      _eps(makeElemArg(&elem)) * _p_var->adGradSln(&elem, correct_skewness)(_index);
  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  _rc_uo.addToA(&elem, _index, strong_residual);
  processResidual(strong_residual * _assembly.elementVolume(&elem), dof_number);
#endif
}
