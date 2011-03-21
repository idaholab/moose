#include "AssemblyData.h"

// MOOSE includes
#include "SubProblem.h"
#include "ArbitraryQuadrature.h"
// libMesh
#include "quadrature_gauss.h"

AssemblyData::AssemblyData(MooseMesh & mesh) :
    _mesh(mesh),

    _fe_helper(getFE(FEType(FIRST, LAGRANGE))),
    _qrule(NULL),
    _qrule_volume(NULL),
    _qrule_arbitrary(NULL),
    _q_points(_fe_helper->get_xyz()),
    _JxW(_fe_helper->get_JxW()),

    _fe_face_helper(getFEFace(FEType(FIRST, LAGRANGE))),
    _qrule_face(NULL),
    _q_points_face(_fe_face_helper->get_xyz()),
    _JxW_face(_fe_face_helper->get_JxW()),
    _normals(_fe_face_helper->get_normals()),

    _current_elem(NULL),
    _current_side(0),
    _current_side_elem(NULL),
    _current_node(NULL)
{
}

AssemblyData::~AssemblyData()
{
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    delete it->second;
  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    delete it->second;
  delete _qrule;
  delete _qrule_face;
  delete _current_side_elem;
}

FEBase * &
AssemblyData::getFE(FEType type)
{
  if (!_fe[type])
    _fe[type] = FEBase::build(_mesh.dimension(), type).release();

  return _fe[type];
}

FEBase * &
AssemblyData::getFEFace(FEType type)
{
  if (!_fe_face[type])
    _fe_face[type] = FEBase::build(_mesh.dimension(), type).release();

  return _fe_face[type];
}

void
AssemblyData::createQRules(Order o)
{
  _qrule_volume = new QGauss(_mesh.dimension(), o);
  _qrule_face = new QGauss(_mesh.dimension() - 1, o);
  _qrule_arbitrary = new ArbitraryQuadrature(_mesh.dimension(), o);

  setVolumeQRule(_qrule_volume);
  setFaceQRule(_qrule_face);
}
  
void
AssemblyData::setVolumeQRule(QBase * qrule)
{
  _qrule = qrule;
  
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->attach_quadrature_rule(_qrule);
}

void
AssemblyData::setFaceQRule(QBase * qrule)
{
  _qrule_face = qrule;
  
  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    it->second->attach_quadrature_rule(_qrule_face);
}

void
AssemblyData::reinit(const Elem * elem)
{
  // Make sure the qrule is the right one
  if(_qrule != _qrule_volume)
    setVolumeQRule(_qrule_volume);

  _current_elem = elem;
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->reinit(elem);
}

void
AssemblyData::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  // Make sure the qrule is the right one
  if(_qrule != _qrule_arbitrary)
    setVolumeQRule(_qrule_arbitrary);

  _qrule_arbitrary->setPoints(reference_points);
  
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

  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    it->second->reinit(elem, side);
}

void
AssemblyData::reinit(const Node * node)
{
  _current_node = node;
}

Real
AssemblyData::computeVolume()
{
  Real current_volume = 0;

//  if (_problem.geomType() == Moose::XYZ)
//  {
    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
      current_volume += _JxW[qp];
//  }
//  else if (_problem.geomType() == Moose::CYLINDRICAL)
//  {
//    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
//      current_volume += _q_points[qp](0) * _JxW[qp];
//  }
//  else
//    mooseError("geom_type must either be XYZ or CYLINDRICAL\n");

    return current_volume;
}
