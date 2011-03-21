#include "AssemblyData.h"
#include "SubProblem.h"

namespace Moose
{

AssemblyData::AssemblyData(SubProblem & problem) :
    _problem(problem),
    _fe_helper(getFE(FEType(FIRST, LAGRANGE))),
    _qrule(NULL),
    _q_points(_fe_helper->get_xyz()),
    _normals(_fe_helper->get_normals()),
    _current_elem(NULL),
    _current_side(0),
    _current_side_elem(NULL),
    _current_node(NULL)
{
}

AssemblyData::~AssemblyData()
{
//  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
//    delete it->second;
  delete _current_side_elem;
}

FEBase * &
AssemblyData::getFE(FEType type)
{
  if (!_fe[type])
    _fe[type] = FEBase::build(_problem.mesh().dimension(), type).release();

  return _fe[type];
}

void
AssemblyData::attachQuadratureRule(QBase *qrule)
{
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->attach_quadrature_rule(qrule);
  _qrule = qrule;
}

void
AssemblyData::reinit(const Elem * elem)
{
  _current_elem = elem;
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->reinit(elem);
}

void
AssemblyData::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_side = side;

  if (_current_side_elem)
    delete _current_side_elem;
  _current_side_elem = elem->build_side(side).release();

  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->reinit(elem, side);
}

void
AssemblyData::reinit(const Node * node)
{
  _current_node = node;
}

} // namespace
