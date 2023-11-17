//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SectionDisplacementAverage.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", SectionDisplacementAverage);

InputParameters
SectionDisplacementAverage::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Compute the section displacement vector average in three-dimensions "
                             "given a user-defined definition of the cross section.");
  params.addParam<std::vector<std::string>>("vector_names",
                                            std::vector<std::string>(1, "value"),
                                            "Names of the column vectors in this object.");
  params.addParam<std::vector<SubdomainName>>(
      "block",
      "The list of blocks in which to search for cross sectional nodes to compute the displacement "
      "vector average.");
  params.addRequiredParam<Point>("axis_direction", "Direction of the structural component's axis");
  params.addRequiredParam<Real>("length", "Distance to cross section from the global origin.");
  params.addParam<Real>("tolerance",
                        1.0e-6,
                        "Distance tolerance to identify nodes on the user-defined cross section.");
  return params;
}

SectionDisplacementAverage::SectionDisplacementAverage(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _displaced_mesh(_app.actionWarehouse().displacedMesh()),
    _mesh(_app.actionWarehouse().mesh()),
    _section_displacements(declareVector("section_displacements")),
    _direction(getParam<Point>("axis_direction")),
    _length(getParam<Real>("length")),
    _tolerance(getParam<Real>("tolerance")),
    _number_of_nodes(0)
{
  if (!MooseUtils::absoluteFuzzyEqual(_direction.norm_sq(), 1.0))
    paramError("axis_direction",
               "Axis diretion must have a norm of one and define the direction along which to "
               "locate cross sectional nodes.");
}

void
SectionDisplacementAverage::initialize()
{
  _section_displacements.clear();
  _section_displacements.resize(3);
}

void
SectionDisplacementAverage::finalize()
{
  _communicator.sum(_section_displacements);
  _communicator.sum(_number_of_nodes);

  _section_displacements[0] /= _number_of_nodes;
  _section_displacements[1] /= _number_of_nodes;
  _section_displacements[2] /= _number_of_nodes;
}

void
SectionDisplacementAverage::execute()
{
  if (!_displaced_mesh->getMesh().is_serial())
    paramError("block", "The variable section average requires a serial mesh.");

  _block_ids = _displaced_mesh->getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"));

  auto * active_nodes = _mesh->getActiveSemiLocalNodeRange();

  Point average_cross_sectional_displacement(0, 0, 0);
  const NumericVector<Number> & _sol = *_sys.currentSolution();

  for (const auto & node : *active_nodes)
  {
    const std::set<SubdomainID> & node_blk_ids = _displaced_mesh->getNodeBlockIds(*node);

    for (const auto i : _block_ids)
      // If not contained in user blocks, continue
      if (node_blk_ids.find(i) != node_blk_ids.end())
      {
        // Check if node is close enough to user-prescribed plane
        if (distancePointPlane(*node, _direction, _length) > _tolerance)
          continue;
        // Check node proc id is our processor id
        if ((*node).processor_id() != processor_id())
          continue;

        // Retrieve displacement variables
        // x
        const MooseVariable & disp_x_variable = _sys.getFieldVariable<Real>(_tid, "disp_x");
        const auto disp_x_num = disp_x_variable.number();
        average_cross_sectional_displacement(0) +=
            _sol(node->dof_number(_sys.number(), disp_x_num, 0));

        // y
        const MooseVariable & disp_y_variable = _sys.getFieldVariable<Real>(_tid, "disp_y");
        const auto disp_y_num = disp_y_variable.number();
        average_cross_sectional_displacement(1) +=
            _sol(node->dof_number(_sys.number(), disp_y_num, 0));

        // z
        const MooseVariable & disp_z_variable = _sys.getFieldVariable<Real>(_tid, "disp_z");
        const auto disp_z_num = disp_z_variable.number();
        average_cross_sectional_displacement(2) +=
            _sol(node->dof_number(_sys.number(), disp_z_num, 0));

        _number_of_nodes++;
      }
  }

  _section_displacements[0] = average_cross_sectional_displacement(0);
  _section_displacements[1] = average_cross_sectional_displacement(1);
  _section_displacements[2] = average_cross_sectional_displacement(2);

  Moose::out << "Final disp vector: " << _section_displacements[0] << " "
             << _section_displacements[1] << " " << _section_displacements[2] << "\n";
}

Real
SectionDisplacementAverage::distancePointPlane(const Node & node,
                                               const Point & axis_direction,
                                               const Real length) const
{
  // length is distance from (0,0,0) to plane
  return std::abs(axis_direction(0) * node(0) + axis_direction(1) * node(1) +
                  axis_direction(2) * node(2) - length) /
         std::sqrt(axis_direction * axis_direction);
}
