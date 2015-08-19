/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FailureIndexPD.h"
#include "SymmTensor.h"
#include "FEProblem.h"
#include <cmath>
#include <algorithm>
#include <set>

template<>
InputParameters validParams<FailureIndexPD>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  return params;
}

FailureIndexPD :: FailureIndexPD(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _bond_status_old(getMaterialPropertyOld<Real>("bond_status" + getParam<std::string>("appended_property_name")))
{
}

FailureIndexPD::~FailureIndexPD()
{
}


void
FailureIndexPD::initialize()
{
  _IntactBonds.clear();
  _BondsNumPerNode.clear();
}

void
FailureIndexPD::execute()
{
  int nodeid0,nodeid1;
  const Node* const node0=_current_elem->get_node(0);
  const Node* const node1=_current_elem->get_node(1);
  nodeid0 = node0->id();
  nodeid1 = node1->id();

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp )
  {
    _IntactBonds[nodeid0] += _bond_status_old[0];
    _IntactBonds[nodeid1] += _bond_status_old[0];
    _BondsNumPerNode[nodeid0] += 1.0;
    _BondsNumPerNode[nodeid1] += 1.0;
  }
}


void
FailureIndexPD::threadJoin(const UserObject & u )
{
  //const FailureIndexPD & sp = dynamic_cast<const FailureIndexPD &>(u);
  //for ( std::map<std::pair<unsigned int, unsigned int>,Real>::const_iterator it = sp._dist.begin();
  //      it != sp._dist.end();
  //      ++it )
  //  _dist[it->first] = it->second;
  //for ( std::map<std::pair<unsigned int, unsigned int>,Real>::const_iterator it = sp._value.begin();
  //      it != sp._value.end();
  //      ++it )
  //  _value[it->first] = it->second;
}

void
FailureIndexPD::finalize()
{
}

Real
FailureIndexPD::ComputeFailureIndex(unsigned int nodeid) const
{
  std::map<unsigned int,Real>::const_iterator ibit = _IntactBonds.find(nodeid);
  if (ibit == _IntactBonds.end())
    mooseError("Couldn't find.");
  std::map<unsigned int,Real>::const_iterator bnit = _BondsNumPerNode.find(nodeid);
  if (bnit == _BondsNumPerNode.end())
    mooseError("Couldn't find.");
  return 1.0 - ibit->second/bnit->second;
}
