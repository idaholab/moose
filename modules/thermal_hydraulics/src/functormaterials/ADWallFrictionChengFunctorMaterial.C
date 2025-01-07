//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallFrictionChengFunctorMaterial.h"
#include "WallFrictionModels.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionChengFunctorMaterial);

InputParameters
ADWallFrictionChengFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("f_D", "Darcy friction factor material property");
  params.addParam<MooseFunctorName>("rho", FlowModelSinglePhase::DENSITY, "Density of the fluid");
  params.addParam<MooseFunctorName>("vel", FlowModelSinglePhase::VELOCITY, "Fluid velocity");
  params.addParam<MooseFunctorName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MooseFunctorName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity of the fluid");
  params.addRequiredRangeCheckedParam<Real>(
      "PoD", "PoD>=1.0", "The Pitch-to-diameter ratio value being assigned into the property");
  MooseEnum bundle_array("SQUARE HEXAGONAL");
  params.addRequiredParam<MooseEnum>(
      "bundle_array", bundle_array, "The type of the rod bundle array");
  MooseEnum subchannel_type("INTERIOR EDGE CORNER");
  params.addRequiredParam<MooseEnum>(
      "subchannel_type", subchannel_type, "The type of subchannel to be considered");
  params.addClassDescription("Computes wall friction factor using the Cheng-Todreas correlation "
                             "for interior, edge and corner channels.");
  return params;
}

ADWallFrictionChengFunctorMaterial::ADWallFrictionChengFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _f_D_name(getParam<MooseFunctorName>("f_D")),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _mu(getFunctor<ADReal>("mu")),
    _PoD(getParam<Real>("PoD")),
    _bundle_array(getParam<MooseEnum>("bundle_array").getEnum<Bundle_array>()),
    _subchannel_type(getParam<MooseEnum>("subchannel_type").getEnum<Subchannel_type>())

{
  addFunctorProperty<ADReal>(
      _f_D_name,
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal Re = THM::Reynolds(1, _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t));
        if (_PoD > 1.5)
        {
          mooseDoOnce(
              mooseWarning("The Cheng-Todreas correlation for the friction factor is valid when "
                           "P/D is between 1.0 "
                           "and 1.5. Be aware that using values out of this range may lead to "
                           "significant errors in your results!"));
        }
        // The Pitch-to-Diameter ratio (PoD) cannot be smaller than 1.0.
        switch (_subchannel_type)
        {
          case Subchannel_type::INTERIOR:
          {
            switch (_bundle_array)
            {
              case Bundle_array::SQUARE:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 26.37;
                    ADReal b = 374.2;
                    ADReal c = -493.9;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.09423;
                    ADReal b = 0.5806;
                    ADReal c = -1.239;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 35.55;
                    ADReal b = 263.7;
                    ADReal c = -190.2;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1339;
                    ADReal b = 0.09059;
                    ADReal c = -0.09926;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              case Bundle_array::HEXAGONAL:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 26;
                    ADReal b = 888.2;
                    ADReal c = -3334;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.09378;
                    ADReal b = 1.398;
                    ADReal c = -8.664;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 62.97;
                    ADReal b = 216.9;
                    ADReal c = -190.2;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1458;
                    ADReal b = 0.03632;
                    ADReal c = -0.03333;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              default:
                mooseError("Invalid 'bundle_array' parameter.");
            }
            break;
          }

          case Subchannel_type::EDGE:
          {
            switch (_bundle_array)
            {
              case Bundle_array::SQUARE:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 26.18;
                    ADReal b = 554.5;
                    ADReal c = -1480;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.09377;
                    ADReal b = 0.8732;
                    ADReal c = -3.341;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 44.40;
                    ADReal b = 256.7;
                    ADReal c = -267.6;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1430;
                    ADReal b = 0.04199;
                    ADReal c = -0.04428;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              case Bundle_array::HEXAGONAL:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 26.18;
                    ADReal b = 554.5;
                    ADReal c = -1480;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.09377;
                    ADReal b = 0.8732;
                    ADReal c = -3.341;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 44.4;
                    ADReal b = 256.7;
                    ADReal c = -267.6;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1430;
                    ADReal b = 0.04199;
                    ADReal c = -0.04428;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              default:
                mooseError("Invalid 'bundle_array' parameter.");
            }
            break;
          }

          case Subchannel_type::CORNER:
          {
            switch (_bundle_array)
            {
              case Bundle_array::SQUARE:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 28.62;
                    ADReal b = 715.9;
                    ADReal c = -2807;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.09755;
                    ADReal b = 1.127;
                    ADReal c = -6.304;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 58.83;
                    ADReal b = 160.7;
                    ADReal c = -203.5;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1452;
                    ADReal b = 0.02681;
                    ADReal c = -0.03411;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              case Bundle_array::HEXAGONAL:
              {
                if (_PoD < 1.1)
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 26.98;
                    ADReal b = 1636;
                    ADReal c = -10050;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1004;
                    ADReal b = 1.625;
                    ADReal c = -11.85;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                else
                {
                  if (Re <= 2100)
                  {
                    ADReal a = 87.26;
                    ADReal b = 38.59;
                    ADReal c = -55.15;
                    ADReal n = 1;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                  else
                  {
                    ADReal a = 0.1499;
                    ADReal b = 0.006706;
                    ADReal c = -0.009567;
                    ADReal n = 0.18;
                    const ADReal f_F =
                        WallFriction::FanningFrictionFactorCheng(Re, a, b, c, n, _PoD);
                    return WallFriction::DarcyFrictionFactor(f_F);
                  }
                }
                break;
              }
              default:
                mooseError("Invalid 'bundle_array' parameter.");
            }
            break;
          }
          default:
            mooseError("Invalid suchannel type");
        }
      });
}
