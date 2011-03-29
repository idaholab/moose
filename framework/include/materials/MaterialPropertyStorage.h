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

#ifndef MATERIALPROPERTYSTORAGE_H
#define MATERIALPROPERTYSTORAGE_H

#include "HashMap.h"
#include "MaterialProperty.h"

/// Stores the material properties computed by materials
/// Thread-safe
///
class MaterialPropertyStorage
{
public:
  MaterialPropertyStorage();
  virtual ~MaterialPropertyStorage();

  void shift();

  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & props() { return *_props_elem; }
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & propsOld() { return *_props_elem_old; }
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & propsOlder() { return *_props_elem_older; }

protected:
  // indexing: [element][side]->material_properties
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem;
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem_old;
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem_older;
};


#endif /* MATERIALPROPERTYSTORAGE_H */
