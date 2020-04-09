//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EulerAngleProvider2RGBAux.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"
#include "Euler2RGB.h"
#include "EBSDReader.h"

registerMooseObject("PhaseFieldApp", EulerAngleProvider2RGBAux);

InputParameters
EulerAngleProvider2RGBAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Output RGB representation of crystal orientation from user object to "
                             "an AuxVariable. The entire domain must have the same crystal "
                             "structure.");
  params.addParam<unsigned int>("phase", "The phase to use for all queries.");
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
    _phase(isParamValid("phase") ? getParam<unsigned int>("phase") : libMesh::invalid_uint),
    _sd(getParam<MooseEnum>("sd")),
    _xtal_class(getParam<MooseEnum>("crystal_structure")),
    _output_type(getParam<MooseEnum>("output_type")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _ebsd_reader(isParamValid("phase") ? dynamic_cast<const EBSDReader *>(&_euler) : nullptr),
    _grain_tracker(dynamic_cast<const GrainTrackerInterface &>(getUserObjectBase("grain_tracker"))),
    _no_grain_color(getParam<Point>("no_grain_color"))
{
}

unsigned int
EulerAngleProvider2RGBAux::getNumGrains() const
{
  if (_phase != libMesh::invalid_uint)
    return _ebsd_reader->getGrainNum(_phase);
  else
    return _euler.getGrainNum();
}

void
EulerAngleProvider2RGBAux::precalculateValue()
{
  const auto grain_id =
      _grain_tracker.getEntityValue(isNodal() ? _current_node->id() : _current_elem->id(),
                                    FeatureFloodCount::FieldType::UNIQUE_REGION,
                                    0);

  // Recover Euler angles for current grain and assign correct RGB value either
  // from Euler2RGB or from _no_grain_color
  Point RGB;
  if (grain_id < 0)
    RGB = _no_grain_color;
  else
  {
    /* The grain index retrieved from FeatureFloodCount is the "global_id" unless
       the "phase" option is used in the simulation.  For the phase dependent case,
       the returned grain index is the "local_id." This must be converted to a
       "global_id" using the getGlobalID function of EBSDREader before the Euler
       Angles are retrieved. */

    auto global_id =
        _phase != libMesh::invalid_uint ? _ebsd_reader->getGlobalID(_phase, grain_id) : grain_id;
    const auto num_grns = getNumGrains();
    if (global_id > num_grns)
      mooseError(" global_id ", global_id, " out of index range");

    // Retrieve Euler Angle values from the EulerAngleProvider
    const RealVectorValue & angles = _euler.getEulerAngles(global_id);

    // Convert Euler Angle values to RGB colorspace for visualization purposes
    RGB = euler2RGB(_sd,
                    angles(0) / 180.0 * libMesh::pi,
                    angles(1) / 180.0 * libMesh::pi,
                    angles(2) / 180.0 * libMesh::pi,
                    1.0,
                    _xtal_class);
  }

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
