/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SubProblemInterface.h"
#include "Conversion.h"

void
SubProblemInterface::storeMatPropName(unsigned int block_id, const std::string & name)
{
  _map_material_props[block_id].insert(name);
}

void
SubProblemInterface::checkMatProp(unsigned int block_id, const std::string & name)
{
  std::map<unsigned int, std::set<std::string> >::iterator it;
  if ((it = _map_material_props.find(block_id)) != _map_material_props.end())
  {
    std::set<std::string>::iterator jt;
    if ((jt = (*it).second.find(name)) == (*it).second.end())
      mooseError("Material property '" + name + "' is not defined on block " + Moose::stringify(block_id));
  }
  else
  {
    mooseError("No material defined on block " + Moose::stringify(block_id));
  }
}
