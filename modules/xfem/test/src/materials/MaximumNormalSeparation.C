/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaximumNormalSeparation.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "ElementPairLocator.h"

registerMooseObject("XFEMTestApp", MaximumNormalSeparation);

InputParameters
MaximumNormalSeparation::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute the separation between neighboring elements");
  params.addRequiredCoupledVar("displacements", "Names of the displacement variables to couple");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same block");
  return params;
}

MaximumNormalSeparation::MaximumNormalSeparation(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _max_normal_separation(declareProperty<Real>(_base_name + "max_normal_separation")),
    _max_normal_separation_old(getMaterialPropertyOld<Real>(_base_name + "max_normal_separation")),
    _disp(coupledValues("displacements")),
    _disp_neighbor(coupledNeighborValues("displacements"))
{
}

void
MaximumNormalSeparation::resetQpProperties()
{
  _max_normal_separation[_qp] = 0.0;
}

void
MaximumNormalSeparation::computeQpProperties()
{
  Point interface_normal(0, 0, 0);
  Real normal_distance = _max_normal_separation_old[_qp];

  GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * element_pair_locators = nullptr;
  element_pair_locators = &geom_search_data._element_pair_locators;

  std::pair<const Elem *, const Elem *> elem_pair =
      std::make_pair(_current_elem, _assembly.neighbor());

  std::pair<const Elem *, const Elem *> elem_pair2 =
      std::make_pair(_assembly.neighbor(), _current_elem);

  for (const auto & it : *element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    // go over pair elements
    const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
        elem_pair_loc.getElemPairs();

    std::list<std::pair<const Elem *, const Elem *>>::const_iterator findIter =
        std::find(elem_pairs.begin(), elem_pairs.end(), elem_pair);

    std::list<std::pair<const Elem *, const Elem *>>::const_iterator findIter2 =
        std::find(elem_pairs.begin(), elem_pairs.end(), elem_pair2);

    if (findIter != elem_pairs.end())
    {
      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(elem_pair);
      interface_normal = info._elem1_normal;
      normal_distance = interface_normal(0) * ((*_disp_neighbor[0])[_qp] - (*_disp[0])[_qp]) +
                        interface_normal(1) * ((*_disp_neighbor[1])[_qp] - (*_disp[1])[_qp]);
    }
    else if (findIter2 != elem_pairs.end())
    {
      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(elem_pair2);
      interface_normal = info._elem2_normal;
      // mooseError("element pair found in opposite way");
      normal_distance = interface_normal(0) * ((*_disp_neighbor[0])[_qp] - (*_disp[0])[_qp]) +
                        interface_normal(1) * ((*_disp_neighbor[1])[_qp] - (*_disp[1])[_qp]);
    }
    else
      mooseError("element pair is not found.");
  }

  if (normal_distance > _max_normal_separation_old[_qp])
    _max_normal_separation[_qp] = normal_distance;
}

void
MaximumNormalSeparation::initQpStatefulProperties()
{
  _max_normal_separation[_qp] = 0.0;
}
