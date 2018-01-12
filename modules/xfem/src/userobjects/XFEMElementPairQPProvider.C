/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMElementPairQPProvider.h"
#include "GeometricSearchData.h"

template <>
InputParameters
validParams<XFEMElementPairQPProvider>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Set up a list of discrete point to evaluate material properties on "
                             "using the XFEMMaterialManager");
  return params;
}

XFEMElementPairQPProvider::XFEMElementPairQPProvider(const InputParameters & parameters)
  : GeneralUserObject(parameters), ElementPairQPProvider()
{
}

void
XFEMElementPairQPProvider::initialSetup()
{
  GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
  _element_pair_locators = &geom_search_data._element_pair_locators;
}

void
XFEMElementPairQPProvider::timestepSetup()
{
  // We have to clear map because old element in the map might be replaced with new one when XFEM
  // modifies mesh. Not efficient but easy way to do is the refresh map at every timestep. The
  // ordering of quadature points for each element needs to be remained, otherwise the
  // XFEMElemPairMaterialManager will fetch the wrong item.

  _extra_qp_map.clear();
  _elem_pair_map.clear();

  for (const auto & it : *_element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    // go over pair elements
    const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
        elem_pair_loc.getElemPairs();

    for (const auto & pr : elem_pairs)
    {
      const Elem * elem1 = pr.first;
      const Elem * elem2 = pr.second;

      if (elem1->processor_id() != processor_id())
        continue;

      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(pr);

      for (unsigned int i = 0; i < (info._elem1_constraint_q_point).size(); ++i)
        _extra_qp_map[std::min(elem1->unique_id(), elem2->unique_id())].push_back(
            (info._elem1_constraint_q_point)[i]);

      _elem_pair_map[std::min(elem1->unique_id(), elem2->unique_id())] = pr;
    }
  }
}
