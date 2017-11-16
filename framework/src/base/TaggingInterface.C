/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TaggingInterface.h"
#include "Conversion.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<TaggingInterface>()
{
  InputParameters params = emptyInputParameters();

  // These are the default names for tags, but users will be able to add their own
  MultiMooseEnum vtags("nontime time", "nontime", true);
  MultiMooseEnum mtags("nontime time", "nontime", true);

  params.addParam<MultiMooseEnum>(
      "vector_tags", vtags, "The tag for the vectors this Kernel should fill");

  params.addParam<MultiMooseEnum>(
      "matrix_tags", mtags, "The tag for the matrices this Kernel should fill");

  params.addParamNamesToGroup("vector_tags matrix_tags", "Advanced");

  return params;
}

TaggingInterface::TaggingInterface(SubProblem & subproblem, const MooseObject & moose_object)
  : _subproblem(subproblem), _moose_object(moose_object)
{
  const InputParameters & parameters = moose_object.parameters();
  auto & vector_tag_names = parameters.get<MultiMooseEnum>("vector_tags");

  if (!vector_tag_names.isValid())
    mooseError("MUST provide at least one vector_tag for Kernel: ", moose_object.name());

  for (auto & vector_tag_name : vector_tag_names)
  {
    if (!_subproblem.vectorTagExists(vector_tag_name.name()))
      mooseError("Kernel, ",
                 moose_object.name(),
                 ", was assigned an invalid vector_tag: '",
                 vector_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _vector_tags.insert(_subproblem.getVectorTag(vector_tag_name));
  }

  auto & matrix_tag_names = parameters.get<MultiMooseEnum>("matrix_tags");

  if (!matrix_tag_names.isValid())
    mooseError("MUST provide at least one matrix_tag for Kernel: ", moose_object.name());

  for (auto & matrix_tag_name : matrix_tag_names)
  {
    if (!_subproblem.matrixTagExists(matrix_tag_name.name()))
      mooseError("Kernel, ",
                 moose_object.name(),
                 ", was assigned an invalid matrix_tag: '",
                 matrix_tag_name,
                 "'.  If this is a TimeKernel then this may have happened because you didn't "
                 "specify a Transient Executioner.");

    _matrix_tags.insert(_subproblem.getMatrixTag(matrix_tag_name));
  }
}

void
TaggingInterface::addVectorTag(TagName & tag_name)
{
  if (!_subproblem.vectorTagExists(tag_name))
    mooseError("Vector tag ", tag_name, " does not exsit in system");

  _vector_tags.insert(_subproblem.getVectorTag(tag_name));
}

void
TaggingInterface::addMatrixTag(TagName & tag_name)
{
  if (!_subproblem.matrixTagExists(tag_name))
    mooseError("Matrix tag ", tag_name, " does not exsit in system");

  _matrix_tags.insert(_subproblem.getMatrixTag(tag_name));
}

void
TaggingInterface::addVectorTag(TagID tag_id)
{
  if (!_subproblem.vectorTagExists(tag_id))
    mooseError("Vector tag ", tag_id, " does not exsit in system");

  _vector_tags.insert(tag_id);
}

void
TaggingInterface::addMatrixTag(TagID tag_id)
{
  if (!_subproblem.matrixTagExists(tag_id))
    mooseError("Matrix tag ", tag_id, " does not exsit in system");

  _matrix_tags.insert(tag_id);
}

TaggingInterface::~TaggingInterface() {}
