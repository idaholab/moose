#include "DiracKernelData.h"

#include "ArbitraryQuadrature.h"

DiracKernelData::DiracKernelData() :
    _arbitrary_qrule(NULL)
{
}

DiracKernelData::~DiracKernelData()
{
}

void
DiracKernelData::setPoints(const std::vector<Point> & physical_points, const std::vector<Point> & mapped_points)
{
  _current_points = physical_points;
  _arbitrary_qrule->setPoints(mapped_points);
}

void
DiracKernelData::init()
{
//  _arbitrary_qrule = new ArbitraryQuadrature(_moose_system.getDim(), _moose_system._max_quadrature_order);
//  _qrule = _arbitrary_qrule;
  
//  QuadraturePointData::init();
}

void
DiracKernelData::reinit(const NumericVector<Number>& soln, const Elem * elem)
{
//  QuadraturePointData::reinit(soln, elem);
}
