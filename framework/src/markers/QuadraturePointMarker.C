//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadraturePointMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

InputParameters
QuadraturePointMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params += MaterialPropertyInterface::validParams();
  MooseEnum third_state("DONT_MARK=-1 COARSEN DO_NOTHING REFINE", "DONT_MARK");
  params.addParam<MooseEnum>(
      "third_state",
      third_state,
      "The Marker state to apply to values falling in-between the coarsen and refine thresholds.");

  params.addParam<bool>("invert",
                        false,
                        "If this is true then values _below_ 'refine' will be "
                        "refined and _above_ 'coarsen' will be coarsened.");
  params.addRequiredParam<VariableName>("variable",
                                        "The values of this variable will be compared "
                                        "to 'refine' and 'coarsen' to see what should "
                                        "be done with the element");
  return params;
}

QuadraturePointMarker::QuadraturePointMarker(const InputParameters & parameters)
  : Marker(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    _u(mooseVariableField().sln()),
    _qrule(_assembly.qRule()),
    _q_point(_assembly.qPoints()),
    _qp(0),
    _third_state(getParam<MooseEnum>("third_state").getEnum<MarkerValue>())
{
  addMooseVariableDependency(&mooseVariableField());
}

Marker::MarkerValue
QuadraturePointMarker::computeElementMarker()
{
  MarkerValue current_mark = DONT_MARK;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    MarkerValue new_mark = computeQpMarker();

    current_mark = std::max(current_mark, new_mark);
  }

  return current_mark;
}
