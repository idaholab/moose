#ifndef ASSEMBLYDATA_H_
#define ASSEMBLYDATA_H_

#include "fe.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"

namespace Moose
{

class SubProblem;

class AssemblyData
{
public:
  AssemblyData(SubProblem & problem);
  virtual ~AssemblyData();

  FEBase * & getFE(FEType type);

  QBase * & qRule() { return _qrule; }
  const std::vector<Point> & qPoints() { return _q_points; }
  const std::vector<Point> & normals() { return _normals; }

  const Elem * & elem() { return _current_elem; }
  unsigned int & side() { return _current_side; }
  const Elem * & sideElem() { return _current_side_elem; }

  const Node * & node() { return _current_node; }

  void attachQuadratureRule(QBase *qrule);

  void reinit(const Elem * elem);
  void reinit(const Elem * elem, unsigned int side);
  void reinit(const Node * node);

protected:
  SubProblem & _problem;
  std::map<FEType, FEBase *> _fe;               /// types of finite elements

  FEBase * _fe_helper;                          /// helper object for transforming coordinates
  QBase * _qrule;
  const std::vector<Point> & _q_points;
  const std::vector<Point> & _normals;          /// Normal vectors at the quadrature points.


  const Elem * _current_elem;                   // The current "element" we are currently on.
  unsigned int _current_side;
  const Elem * _current_side_elem;              // The current "element" making up the side we are currently on.

  const Node * _current_node;


};

} // namespace

#endif /* ASSEMBLYDATA_H_ */
