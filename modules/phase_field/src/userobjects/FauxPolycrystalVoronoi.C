//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FauxPolycrystalVoronoi.h"
#include "IndirectSort.h"
#include "MooseRandom.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NonlinearSystemBase.h"
#include "DelimitedFileReader.h"

registerMooseObject("PhaseFieldApp", FauxPolycrystalVoronoi);

InputParameters
FauxPolycrystalVoronoi::validParams()
{
  InputParameters params = PolycrystalVoronoi::validParams();
  params.addClassDescription("Random Voronoi tessellation polycrystal when the number of order "
                             "parameters equal to the number of grains");
  return params;
}

FauxPolycrystalVoronoi::FauxPolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalVoronoi(parameters)
{
  if (_grain_num != _op_num)
    paramError("op_num", "The number of order parameters has to equal to the number of grains");
}

void
FauxPolycrystalVoronoi::initialSetup()
{
  /**
   * For polycrystal ICs we need to assume that each of the variables has the same periodicity.
   * Since BCs are handled elsewhere in the system, we'll have to check this case explicitly.
   */
  if (_op_num < 1)
    mooseError("No coupled variables found");

  for (unsigned int dim = 0; dim < _dim; ++dim)
  {
    bool first_variable_value = _mesh.isTranslatedPeriodic(_vars[0]->number(), dim);

    for (unsigned int i = 1; i < _vars.size(); ++i)
      if (_mesh.isTranslatedPeriodic(_vars[i]->number(), dim) != first_variable_value)
        mooseError("Coupled polycrystal variables differ in periodicity");
  }
}

void
FauxPolycrystalVoronoi::execute()
{
  precomputeGrainStructure();
}

void
FauxPolycrystalVoronoi::finalize()
{
  _grain_to_op.clear();

  for (auto grain = decltype(_grain_num)(0); grain < _grain_num; grain++)
    _grain_to_op.emplace_hint(_grain_to_op.end(), grain, grain);
}
