#ifndef ASSEMBLYDATA_H_
#define ASSEMBLYDATA_H_

#include "fe.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"

namespace Moose
{

class Mesh;

class AssemblyData
{
public:
  AssemblyData(Mesh & mesh);
  virtual ~AssemblyData();

  FEBase * & getFE(FEType type);
  QBase * & qRule() { return _qrule; }
  const std::vector<Point> & qPoints() { return _q_points; }
  const std::vector<Real> & JxW() { return _JxW; }

  FEBase * & getFEFace(FEType type);
  QBase * & qRuleFace() { return _qrule_face; }
  const std::vector<Point> & qPointsFace() { return _q_points_face; }
  const std::vector<Real> & JxWFace() { return _JxW_face; }

  const std::vector<Point> & normals() { return _normals; }

  const Elem * & elem() { return _current_elem; }
  unsigned int & side() { return _current_side; }
  const Elem * & sideElem() { return _current_side_elem; }

  const Node * & node() { return _current_node; }

  void attachQuadratureRule(Order o);

  void reinit(const Elem * elem);
  void reinit(const Elem * elem, unsigned int side);
  void reinit(const Node * node);

  Real computeVolume();

protected:
  Mesh & _mesh;

  std::map<FEType, FEBase *> _fe;               /// types of finite elements
  FEBase * _fe_helper;                          /// helper object for transforming coordinates
  QBase * _qrule;
  const std::vector<Point> & _q_points;
  const std::vector<Real> & _JxW;

  std::map<FEType, FEBase *> _fe_face;          /// types of finite elements
  FEBase * _fe_face_helper;                          /// helper object for transforming coordinates
  QBase * _qrule_face;
  const std::vector<Point> & _q_points_face;
  const std::vector<Real> & _JxW_face;
  const std::vector<Point> & _normals;          /// Normal vectors at the quadrature points.


  const Elem * _current_elem;                   // The current "element" we are currently on.
  unsigned int _current_side;
  const Elem * _current_side_elem;              // The current "element" making up the side we are currently on.

  const Node * _current_node;


};

} // namespace

#endif /* ASSEMBLYDATA_H_ */
