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

template <>
InputParameters
validParams<MaximumNormalSeparation>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("");
  params.addRequiredCoupledVar("disp_x", "Name of the variable to couple");
  params.addRequiredCoupledVar("disp_y", "Name of the variable to couple");
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
    _disp_x(coupledValue("disp_x")),
    _disp_x_neighbor(coupledNeighborValue("disp_x")),
    _disp_y(coupledValue("disp_y")),
    _disp_y_neighbor(coupledNeighborValue("disp_y"))
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
  Real normal_distance;

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
      // std::cout << "find current elem id = " << (findIter->first)->id()
      //           << ", find neighbor id = " << (findIter->second)->id() << std::endl;
      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(elem_pair);
      interface_normal = info._elem1_normal;
      // std::cout << "normal = " << info._elem1_normal << std::endl;
      normal_distance = interface_normal(0) * (_disp_x_neighbor[_qp] - _disp_x[_qp]) +
                        interface_normal(1) * (_disp_y_neighbor[_qp] - _disp_y[_qp]);
    }
    else if (findIter2 != elem_pairs.end())
    {
      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(elem_pair2);
      interface_normal = info._elem2_normal;
      // mooseError("element pair found in opposite way");
      normal_distance = interface_normal(0) * (_disp_x_neighbor[_qp] - _disp_x[_qp]) +
                        interface_normal(1) * (_disp_y_neighbor[_qp] - _disp_y[_qp]);
    }
    else
    {
      mooseError("element pair is not found.");
    }
  }
  // std::cout << "current elem id = " << _current_elem->id()
  //           << ", neighbor id = " << (_assembly.neighbor())->id() << std::endl;

  if (normal_distance > _max_normal_separation_old[_qp])
    _max_normal_separation[_qp] = normal_distance;
  else
    _max_normal_separation[_qp] = _max_normal_separation_old[_qp];
}

void
MaximumNormalSeparation::initQpStatefulProperties()
{
  _max_normal_separation[_qp] = 0.0;
}
