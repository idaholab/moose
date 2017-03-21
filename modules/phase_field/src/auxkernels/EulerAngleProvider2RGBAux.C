/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "EulerAngleProvider2RGBAux.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"
#include "Euler2RGB.h"

template <>
InputParameters
validParams<EulerAngleProvider2RGBAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Output RGB representation of crystal orientation from user object to "
                             "an AuxVariable. The entire domain must have the same crystal "
                             "structure.");
  MooseEnum sd_enum = MooseEnum("100=1 010=2 001=3", "001");
  params.addParam<MooseEnum>("sd", sd_enum, "Reference sample direction");
  MooseEnum structure_enum = MooseEnum(
      "cubic=43 hexagonal=62 tetragonal=42 trigonal=32 orthorhombic=22 monoclinic=2 triclinic=1");
  params.addRequiredParam<MooseEnum>(
      "crystal_structure", structure_enum, "Crystal structure of the material");
  MooseEnum output_types = MooseEnum("red green blue scalar", "scalar");
  params.addParam<MooseEnum>("output_type", output_types, "Type of value that will be outputted");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addParam<Point>(
      "no_grain_color",
      Point(0, 0, 0),
      "RGB value of color used to represent area with no grains, defaults to black");
  return params;
}

EulerAngleProvider2RGBAux::EulerAngleProvider2RGBAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _sd(getParam<MooseEnum>("sd")),
    _xtal_class(getParam<MooseEnum>("crystal_structure")),
    _output_type(getParam<MooseEnum>("output_type")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker")),
    _no_grain_color(getParam<Point>("no_grain_color"))
{
}

void
EulerAngleProvider2RGBAux::precalculateValue()
{
  // ID of unique grain at current point
  const Real grain_id =
      _grain_tracker.getEntityValue((isNodal() ? _current_node->id() : _current_elem->id()),
                                    FeatureFloodCount::FieldType::UNIQUE_REGION,
                                    0);

  // Recover Euler angles for current grain and assign correct
  // RGB value either from euler2RGB or from _no_grain_color
  Point RGB;
  if (grain_id >= 0 && grain_id < _euler.getGrainNum())
  {
    const RealVectorValue & angles = _euler.getEulerAngles(grain_id);
    RGB = euler2RGB(_sd,
                    angles(0) / 180.0 * libMesh::pi,
                    angles(1) / 180.0 * libMesh::pi,
                    angles(2) / 180.0 * libMesh::pi,
                    1.0,
                    _xtal_class);
  }
  else
    RGB = _no_grain_color;

  // Create correct scalar output
  if (_output_type < 3)
    _value = RGB(_output_type);
  else if (_output_type == 3)
  {
    Real RGBint = 0.0;
    for (unsigned int i = 0; i < 3; ++i)
      RGBint = 256 * RGBint + (RGB(i) >= 1 ? 255 : std::floor(RGB(i) * 256.0));

    _value = RGBint;
  }
  else
    mooseError("Incorrect value for output_type in EulerAngleProvider2RGBAux");
}

Real
EulerAngleProvider2RGBAux::computeValue()
{
  return _value;
}
