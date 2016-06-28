/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "OutputRGB.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"
#include "Euler2RGB.h"

template<>
InputParameters validParams<OutputRGB>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Output RGB representation of crystal orientation from user object to an AuxVariable. The entire domain must have the same crystal structure.");
  MooseEnum sd_enum = MooseEnum("100=1 010=2 001=3", "001");
  params.addParam<MooseEnum>("sd", sd_enum, "Reference sample direction");
  MooseEnum structure_enum = MooseEnum("cubic=43 hexagonal=62 tetragonal=42 trigonal=32 orthorhombic=22 monoclinic=2 triclinic=1");
  params.addRequiredParam<MooseEnum>("crystal_structure",structure_enum,"Crystal structure of the material");
  params.addRequiredParam<UserObjectName>("euler_angle_provider", "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_tracker_object", "The GrainTracker UserObject to get values from.");
  return params;
}

OutputRGB::OutputRGB(const InputParameters & parameters) :
    AuxKernel(parameters),
    _sd(getParam<MooseEnum>("sd")),
    _xtal_class(getParam<MooseEnum>("crystal_structure")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker_object"))
{
}

Real
OutputRGB::computeValue()
{
  // ID of unique grain at current point
  const unsigned int grain_id = _grain_tracker.getEntityValue((isNodal() ? _current_node->id() : _current_elem->id()),
                                                              FeatureFloodCount::FieldType::UNIQUE_REGION, 0);
  // Recover euler angles for current grain
  const RealVectorValue angles = _euler.getEulerAngles(grain_id);

  // Return RGB value computed from Euler angles
  return Euler2RGB(_sd, angles(0) / 180.0 * libMesh::pi, angles(1) / 180.0 * libMesh::pi, angles(2) / 180.0 * libMesh::pi, 1.0, _xtal_class);
}
