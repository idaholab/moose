#include "PostprocessorSpatialUserObject.h"

registerMooseObject("MooseApp", PostprocessorSpatialUserObject);

InputParameters
PostprocessorSpatialUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  params.addClassDescription("User object (spatial) that holds a postprocessor value.");
  return params;
}

PostprocessorSpatialUserObject::PostprocessorSpatialUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _value(getPostprocessorValue("postprocessor"))
{
}

void
PostprocessorSpatialUserObject::initialize()
{
}

void
PostprocessorSpatialUserObject::execute()
{
}

void
PostprocessorSpatialUserObject::finalize()
{
}

Real
PostprocessorSpatialUserObject::spatialValue(const Point & /*p*/) const
{
  return _value;
}
