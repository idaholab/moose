//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVVaporRecoilPressureMomentumFluxBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVVaporRecoilPressureMomentumFluxBC);

InputParameters
INSFVVaporRecoilPressureMomentumFluxBC::validParams()
{
  InputParameters params = INSFVFreeSurfaceBC::validParams();
  params.addClassDescription(
      "Imparts a surface recoil force on the momentum equation due to liquid phase evaporation");
  params.addParam<MaterialPropertyName>("rc_pressure", "rc_pressure", "The recoil pressure");
  return params;
}

INSFVVaporRecoilPressureMomentumFluxBC::INSFVVaporRecoilPressureMomentumFluxBC(
    const InputParameters & params)
  : INSFVFreeSurfaceBC(params), _rc_pressure(getFunctor<ADReal>("rc_pressure"))
{
}

void
INSFVVaporRecoilPressureMomentumFluxBC::gatherRCData(const FaceInfo & fi)
{
  _face_info = &fi;
  _face_type = fi.faceType(_var.name());
  const auto strong_resid =
      fi.normal()(_index) * _rc_pressure(singleSidedFaceArg(), determineState());
  addResidualAndJacobian(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
