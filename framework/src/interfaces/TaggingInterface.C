//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TaggingInterface.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Assembly.h"

#include "libmesh/dense_vector.h"

InputParameters
TaggingInterface::validParams()
{

  InputParameters params = emptyInputParameters();

  // These are the default names for tags, but users will be able to add their own
  MultiMooseEnum vtags("nontime time", "nontime", true);
  MultiMooseEnum mtags("nontime system", "system", true);

  params.addParam<MultiMooseEnum>(
      "vector_tags", vtags, "The tag for the vectors this Kernel should fill");

  params.addParam<MultiMooseEnum>(
      "matrix_tags", mtags, "The tag for the matrices this Kernel should fill");

  params.addParam<std::vector<TagName>>("extra_vector_tags",
                                        "The extra tags for the vectors this Kernel should fill");

  params.addParam<std::vector<TagName>>(
      "absolute_value_vector_tags",
      "The tags for the vectors this residual object should fill with the "
      "absolute value of the residual contribution");

  params.addParam<std::vector<TagName>>("extra_matrix_tags",
                                        "The extra tags for the matrices this Kernel should fill");

  params.addParamNamesToGroup(
      "vector_tags matrix_tags extra_vector_tags extra_matrix_tags absolute_value_vector_tags",
      "Tagging");

  return params;
}

TaggingInterface::TaggingInterface(const MooseObject * moose_object)
  : _moose_object(*moose_object),
    _tag_params(_moose_object.parameters()),
    _subproblem(*_tag_params.getCheckedPointerParam<SubProblem *>("_subproblem"))
{
  auto & vector_tag_names = _tag_params.get<MultiMooseEnum>("vector_tags");

  if (!vector_tag_names.isValid())
    mooseError("MUST provide at least one vector_tag for Kernel: ", _moose_object.name());

  for (auto & vector_tag_name : vector_tag_names)
  {
    const TagID vector_tag_id = _subproblem.getVectorTagID(vector_tag_name.name());
    if (_subproblem.vectorTagType(vector_tag_id) != Moose::VECTOR_TAG_RESIDUAL)
      mooseError("Vector tag '",
                 vector_tag_name.name(),
                 "' for Kernel '",
                 _moose_object.name(),
                 "' is not a residual vector tag");
    _vector_tags.insert(vector_tag_id);
  }

  // Add extra vector tags. These tags should be created in the System already, otherwise
  // we can not add the extra tags
  auto & extra_vector_tags = _tag_params.get<std::vector<TagName>>("extra_vector_tags");

  for (auto & vector_tag_name : extra_vector_tags)
  {
    const TagID vector_tag_id = _subproblem.getVectorTagID(vector_tag_name);
    if (_subproblem.vectorTagType(vector_tag_id) != Moose::VECTOR_TAG_RESIDUAL)
      mooseError("Extra vector tag '",
                 vector_tag_name,
                 "' for Kernel '",
                 _moose_object.name(),
                 "' is not a residual vector tag");
    _vector_tags.insert(vector_tag_id);
  }

  // Add absolue value vector tags. These tags should be created in the System already, otherwise
  // we can not add the extra tags
  auto & abs_vector_tags = _tag_params.get<std::vector<TagName>>("absolute_value_vector_tags");

  for (auto & vector_tag_name : abs_vector_tags)
  {
    const TagID vector_tag_id = _subproblem.getVectorTagID(vector_tag_name);
    if (_subproblem.vectorTagType(vector_tag_id) != Moose::VECTOR_TAG_RESIDUAL)
      mooseError("Absolute value vector tag '",
                 vector_tag_name,
                 "' for Kernel '",
                 _moose_object.name(),
                 "' is not a residual vector tag");
    _abs_vector_tags.insert(vector_tag_id);
  }

  auto & matrix_tag_names = _tag_params.get<MultiMooseEnum>("matrix_tags");

  if (!matrix_tag_names.isValid())
    mooseError("MUST provide at least one matrix_tag for Kernel: ", _moose_object.name());

  for (auto & matrix_tag_name : matrix_tag_names)
    _matrix_tags.insert(_subproblem.getMatrixTagID(matrix_tag_name.name()));

  auto & extra_matrix_tags = _tag_params.get<std::vector<TagName>>("extra_matrix_tags");

  for (auto & matrix_tag_name : extra_matrix_tags)
    _matrix_tags.insert(_subproblem.getMatrixTagID(matrix_tag_name));

  _re_blocks.resize(_vector_tags.size());
  _absre_blocks.resize(_abs_vector_tags.size());
  _ke_blocks.resize(_matrix_tags.size());
}

void
TaggingInterface::useVectorTag(const TagName & tag_name)
{
  if (!_subproblem.vectorTagExists(tag_name))
    mooseError("Vector tag ", tag_name, " does not exist in system");

  _vector_tags.insert(_subproblem.getVectorTagID(tag_name));
}

void
TaggingInterface::useMatrixTag(const TagName & tag_name)
{
  if (!_subproblem.matrixTagExists(tag_name))
    mooseError("Matrix tag ", tag_name, " does not exist in system");

  _matrix_tags.insert(_subproblem.getMatrixTagID(tag_name));
}

void
TaggingInterface::useVectorTag(TagID tag_id)
{
  if (!_subproblem.vectorTagExists(tag_id))
    mooseError("Vector tag ", tag_id, " does not exist in system");

  _vector_tags.insert(tag_id);
}

void
TaggingInterface::useMatrixTag(TagID tag_id)
{
  if (!_subproblem.matrixTagExists(tag_id))
    mooseError("Matrix tag ", tag_id, " does not exist in system");

  _matrix_tags.insert(tag_id);
}

void
TaggingInterface::prepareVectorTag(Assembly & assembly, unsigned int ivar)
{
  _re_blocks.resize(_vector_tags.size());
  mooseAssert(_vector_tags.size() >= 1, "we need at least one active tag");
  auto vector_tag = _vector_tags.begin();
  for (MooseIndex(_vector_tags) i = 0; i < _vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _re_blocks[i] = &assembly.residualBlock(ivar, tag._type_id);
  }

  _local_re.resize(_re_blocks[0]->size());

  _absre_blocks.resize(_abs_vector_tags.size());
  vector_tag = _abs_vector_tags.begin();
  for (MooseIndex(_abs_vector_tags) i = 0; i < _abs_vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _absre_blocks[i] = &assembly.residualBlock(ivar, tag._type_id);
  }
}

void
TaggingInterface::prepareVectorTagNeighbor(Assembly & assembly, unsigned int ivar)
{
  _re_blocks.resize(_vector_tags.size());
  mooseAssert(_vector_tags.size() >= 1, "we need at least one active tag");
  auto vector_tag = _vector_tags.begin();
  for (MooseIndex(_vector_tags) i = 0; i < _vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _re_blocks[i] = &assembly.residualBlockNeighbor(ivar, tag._type_id);
  }
  _local_re.resize(_re_blocks[0]->size());

  _absre_blocks.resize(_abs_vector_tags.size());
  vector_tag = _abs_vector_tags.begin();
  for (MooseIndex(_abs_vector_tags) i = 0; i < _abs_vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _absre_blocks[i] = &assembly.residualBlockNeighbor(ivar, tag._type_id);
  }
}

void
TaggingInterface::prepareVectorTagLower(Assembly & assembly, unsigned int ivar)
{
  _re_blocks.resize(_vector_tags.size());
  mooseAssert(_vector_tags.size() >= 1, "we need at least one active tag");
  auto vector_tag = _vector_tags.begin();
  for (MooseIndex(_vector_tags) i = 0; i < _vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _re_blocks[i] = &assembly.residualBlockLower(ivar, tag._type_id);
  }
  _local_re.resize(_re_blocks[0]->size());

  _absre_blocks.resize(_abs_vector_tags.size());
  vector_tag = _abs_vector_tags.begin();
  for (MooseIndex(_abs_vector_tags) i = 0; i < _abs_vector_tags.size(); i++, ++vector_tag)
  {
    const VectorTag & tag = _subproblem.getVectorTag(*vector_tag);
    _absre_blocks[i] = &assembly.residualBlockLower(ivar, tag._type_id);
  }
}

void
TaggingInterface::prepareMatrixTag(Assembly & assembly, unsigned int ivar, unsigned int jvar)
{
  _ke_blocks.resize(_matrix_tags.size());
  mooseAssert(_matrix_tags.size() >= 1, "we need at least one active tag");
  auto mat_vector = _matrix_tags.begin();
  for (MooseIndex(_matrix_tags) i = 0; i < _matrix_tags.size(); i++, ++mat_vector)
    _ke_blocks[i] = &assembly.jacobianBlock(ivar, jvar, *mat_vector);

  _local_ke.resize(_ke_blocks[0]->m(), _ke_blocks[0]->n());
}

void
TaggingInterface::prepareMatrixTagNeighbor(Assembly & assembly,
                                           unsigned int ivar,
                                           unsigned int jvar,
                                           Moose::DGJacobianType type)
{
  _ke_blocks.resize(_matrix_tags.size());
  mooseAssert(_matrix_tags.size() >= 1, "we need at least one active tag");
  auto mat_vector = _matrix_tags.begin();
  for (MooseIndex(_matrix_tags) i = 0; i < _matrix_tags.size(); i++, ++mat_vector)
    _ke_blocks[i] = &assembly.jacobianBlockNeighbor(type, ivar, jvar, *mat_vector);

  _local_ke.resize(_ke_blocks[0]->m(), _ke_blocks[0]->n());
}

void
TaggingInterface::prepareMatrixTagLower(Assembly & assembly,
                                        unsigned int ivar,
                                        unsigned int jvar,
                                        Moose::ConstraintJacobianType type)
{
  _ke_blocks.resize(_matrix_tags.size());
  mooseAssert(_matrix_tags.size() >= 1, "we need at least one active tag");
  auto mat_vector = _matrix_tags.begin();
  for (MooseIndex(_matrix_tags) i = 0; i < _matrix_tags.size(); i++, ++mat_vector)
    _ke_blocks[i] = &assembly.jacobianBlockMortar(type, ivar, jvar, *mat_vector);

  _local_ke.resize(_ke_blocks[0]->m(), _ke_blocks[0]->n());
}

void
TaggingInterface::accumulateTaggedLocalResidual()
{
  for (auto & re : _re_blocks)
    *re += _local_re;
  for (auto & absre : _absre_blocks)
    for (const auto i : index_range(_local_re))
      (*absre)(i) += std::abs(_local_re(i));
}

void
TaggingInterface::assignTaggedLocalResidual()
{
  for (auto & re : _re_blocks)
    *re = _local_re;
  for (auto & absre : _absre_blocks)
    for (const auto i : index_range(_local_re))
      (*absre)(i) = std::abs(_local_re(i));
}

void
TaggingInterface::accumulateTaggedLocalMatrix()
{
  for (auto & ke : _ke_blocks)
    *ke += _local_ke;
}

void
TaggingInterface::assignTaggedLocalMatrix()
{
  for (auto & ke : _ke_blocks)
    *ke = _local_ke;
}

TaggingInterface::~TaggingInterface() {}
