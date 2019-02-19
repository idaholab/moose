#include "NormalizationParameter.h"
#include "MooseError.h"

registerMooseObject("THMApp", NormalizationParameter);

template <>
InputParameters
validParams<NormalizationParameter>()
{
  InputParameters params = validParams<GeneralUserObject>();
  // Constants
  params.addRequiredParam<Real>("M_threshold",
                                "Coefficient for function used in the normalization parameter");
  params.addRequiredParam<Real>("a",
                                "Coefficient for function used in the normalization parameter");
  params.addRequiredParam<PostprocessorName>("avg_vel", "average velocity");
  // Function type
  MooseEnum funct_type_moose_enum("Mach Tanh Sin Shock", "Mach");
  params.addRequiredParam<MooseEnum>(
      "funct_type",
      funct_type_moose_enum,
      "function used in the def. of the normalization of the visc. coeff.");
  return params;
}

NormalizationParameter::NormalizationParameter(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    // Constant
    _M_thres(getParam<Real>("M_threshold")),
    _a(getParam<Real>("a")),
    // Get the names of functions postprocessing velocity and pressure values
    _avg_vel(getPostprocessorValue("avg_vel")),
    // Function type
    _fnct_type(getParam<MooseEnum>("funct_type"))
{
}

void
NormalizationParameter::destroy()
{
}

Real
NormalizationParameter::compute(Real c2, Real vel) const
{
  Real Mach = std::fabs(vel) / std::sqrt(c2);
  Real func, norm;
  switch (_fnct_type)
  {
    case 0: // Mach
      func = std::min(Mach, 1.);
      norm = 0.5 * std::fabs((1. - func) * c2 + func * std::min(c2, vel * vel));
      break;

    case 1: // Tanh
      func = std::tanh(_a * (Mach - _M_thres));
      func += std::fabs(std::tanh(_a * (Mach - _M_thres)));
      func *= 0.5;
      norm = 0.5 * std::fabs((1. - func) * c2 + func * std::min(c2, vel * vel));
      break;

    case 2: // Sin
      if (Mach < _M_thres - _a)
        func = 0.;
      else if (Mach > _M_thres + _a)
        func = 1.;
      else
      {
        func = 1. + (Mach - _M_thres) / _a;
        func += std::sin(libMesh::pi * (Mach - _M_thres) / _a) / libMesh::pi;
        func *= 0.5;
      }
      norm = 0.5 * std::fabs((1. - func) * c2 + func * std::min(c2, vel * vel));
      break;

    case 3: // Shock
      norm = std::max(std::min(c2, vel * vel), _avg_vel * _avg_vel);
      break;

    default:
      mooseError("'",
                 this->name(),
                 "' Undefined function type to compute the normalization "
                 "parameter used in the computation of the visc. coeff.");
  }
  return norm;
}
