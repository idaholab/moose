//Moose includes
#include "QuadrPtData.h"
#include "MooseSystem.h"

//libmesh includes
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

QuadrPtData::QuadrPtData(MooseSystem & moose_system) :
  _moose_system(moose_system)
{
  sizeEverything();
}

QuadrPtData::~QuadrPtData()
{
  for (std::vector<std::map<FEType, FEBase*> >::iterator i = _fe.begin(); i != _fe.end(); ++i)
  {
    for (std::map<FEType, FEBase*>::iterator j = i->begin(); j != i->end(); ++j)
      delete j->second;
  }

  for (std::vector<QGauss *>::iterator i = _qrule.begin(); i != _qrule.end(); ++i)
  {
    delete *i;
  }
}

void
QuadrPtData::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _fe.resize(n_threads);
  _qrule.resize(n_threads);
  _q_point.resize(n_threads);
  _JxW.resize(n_threads);
  _phi.resize(n_threads);
  _dphi.resize(n_threads);
  _d2phi.resize(n_threads);
}
