#include "Diff1Material.h"
#include "libmesh/dense_matrix.h"

template<>
InputParameters validParams<Diff1Material>()
{
  InputParameters params = validParams<Material>();
  params.set<Real>("diff") = 1.0;
  return params;
}

Diff1Material::Diff1Material(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _diff(getParam<Real>("diff")),
    _diffusivity(declareProperty<Real>("diff1")),
    _vprop(declareProperty<std::vector<Real> >("vprop")),
    _matrix_mat(declareProperty<DenseMatrix<Real> >("matrix_mat"))
{
}

void
Diff1Material::computeQpProperties()
{
  _diffusivity[_qp] = _diff;
}
