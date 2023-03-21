//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*repl
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CallTaggedResidualsTest.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "libmesh/petsc_vector.h"

registerMooseObject("MooseTestApp", CallTaggedResidualsTest);

InputParameters
CallTaggedResidualsTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<TagName>>("residual_tags", "");
  return params;
}

CallTaggedResidualsTest::CallTaggedResidualsTest(const InputParameters & parameters)
  : GeneralUserObject(parameters), _nl(_fe_problem.getNonlinearSystemBase())
{
  for (const auto & tag : getParam<std::vector<TagName>>("residual_tags"))
  {
    auto tag_id = _fe_problem.getVectorTagID(tag);
    _tags.push_back(tag_id);
    _tag_vectors.push_back(&_nl.getVector(tag_id));
  }
}

void
CallTaggedResidualsTest::initialSetup()
{
  _saved_vectors.clear();
  for (const auto & vec : _tag_vectors)
    _saved_vectors.push_back(vec->clone());
}

void
CallTaggedResidualsTest::execute()
{
  // Call residual to accumulate all vectors
  _fe_problem.computeResidual(*_nl.currentSolution(), _nl.RHS());

  // Copy into saved vectors
  for (const auto & i : index_range(_tags))
    *_saved_vectors[i] = *_tag_vectors[i];

  // Call residual for each tag individually
  for (const auto & i : index_range(_tags))
  {
    _fe_problem.computeResidualTags({_tags[i]});

    // Check to see if the vectors match
    std::set<std::size_t> diff;
    for (const auto & j : index_range(_tags))
      if (_saved_vectors[j]->compare(*_tag_vectors[j]) != -1)
        diff.insert(j);

    // Print error
    if (!diff.empty())
    {
      std::stringstream ss;
      ss << "The following vectors have different values after residual call with "
         << _fe_problem.vectorTagName(_tags[i]) << ":\n";
      for (const auto & j : diff)
      {
        ss << "\t" << _fe_problem.vectorTagName(_tags[j]) << "\n\tindex\tvalue\tsaved\tdiff\n";

        std::vector<Number> tag_local;
        _tag_vectors[j]->localize(tag_local);
        std::vector<Number> saved_local;
        _saved_vectors[j]->localize(saved_local);
        for (const auto & ind : index_range(tag_local))
          ss << "\t" << ind << "\t" << tag_local[ind] << "\t" << saved_local[ind] << "\t"
             << (tag_local[ind] - saved_local[ind]) << "\n";

        ss << "\n";
      }
      mooseError(ss.str());
    }
  }
}
