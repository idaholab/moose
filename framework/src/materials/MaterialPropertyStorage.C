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

#include "MaterialPropertyStorage.h"

MaterialPropertyStorage::MaterialPropertyStorage()
{
  _props_elem       = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_old   = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_older = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
}

MaterialPropertyStorage::~MaterialPropertyStorage()
{
  {
    HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >::iterator i;
    for (i = _props_elem->begin(); i != _props_elem->end(); ++i)
    {
      HashMap<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem;

    for (i = _props_elem_old->begin(); i != _props_elem_old->end(); ++i)
    {
      HashMap<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem_old;

    for (i = _props_elem_older->begin(); i != _props_elem_older->end(); ++i)
    {
      HashMap<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem_older;
  }
}

void
MaterialPropertyStorage::shift()
{
  // shift the properties back in time and reuse older for current (save reallocations etc.)
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * tmp = _props_elem_older;

  _props_elem_older = _props_elem_old;
  _props_elem_old = _props_elem;
  _props_elem = tmp;
}
