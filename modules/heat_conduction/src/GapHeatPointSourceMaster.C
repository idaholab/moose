#include "GapHeatPointSourceMaster.h"
#include "SystemBase.h"

template<>
InputParameters validParams<GapHeatPointSourceMaster>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<unsigned int>("boundary", "The master boundary");
  params.addRequiredParam<unsigned int>("slave", "The slave boundary");
  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

GapHeatPointSourceMaster::GapHeatPointSourceMaster(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("boundary"), getParam<unsigned int>("slave"))),
   _slave_flux(_sys.getVector("pellet_flux"))
{}

void
GapHeatPointSourceMaster::addPoints()
{
  point_to_info.clear();

  _slave_flux.close();
  _slave_flux.localize(_localized_slave_flux);

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for(; it!=end; ++it)
  {
    unsigned int slave_node_num = it->first;
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if(!pinfo)
      continue;

    Node * node = pinfo->_node;

    addPoint(pinfo->_elem, pinfo->_closest_point);
    point_to_info[pinfo->_closest_point] = pinfo;
  }
}

Real
GapHeatPointSourceMaster::computeQpResidual()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
  long int dof_number = node->dof_number(0, _var.number(), 0);

  return -_phi[_i][_qp] * _localized_slave_flux[dof_number];
}

Real
GapHeatPointSourceMaster::computeQpJacobian()
{
  return 0;
}
