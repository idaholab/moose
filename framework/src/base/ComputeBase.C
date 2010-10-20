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

//Moose Includes
#include "ComputeBase.h"
#include "MooseSystem.h"

//libMesh includes
//#include "numeric_vector.h"
//#include "dense_vector.h"
//#include "petsc_matrix.h"
//#include "dof_map.h"
//#include "mesh.h"
#include "boundary_info.h"

#include <vector>

ComputeBase::ComputeBase(MooseSystem &sys) :
  _moose_system(sys)
{
}

void ComputeBase::operator () (const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  pre();

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();
  ConstElemRange::const_iterator el = range.begin();
  for (el = range.begin() ; el != range.end(); ++el)
  {
    const Elem* elem = *el;
    unsigned int cur_subdomain = elem->subdomain_id();

    preElement(elem);

    if(cur_subdomain != subdomain)
    {
      subdomain = cur_subdomain;
      onDomainChanged(subdomain);
    }

    onElement(elem);

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<short int> boundary_ids = _moose_system.getMesh()->boundary_info->boundary_ids (elem, side);

      if (boundary_ids.size() > 0)
      {
        for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(elem, side, *it);
      }

      if (elem->neighbor(side) != NULL)
        onInternalSide(elem, side);
    } // sides

    postElement(elem);
  } // range

  post();
}

void
ComputeBase::pre()
{

}

void
ComputeBase::post()
{

}

void
ComputeBase::preElement(const Elem * /*elem*/)
{
}

void
ComputeBase::onElement(const Elem * /*elem*/)
{
}

void
ComputeBase::postElement(const Elem * /*elem*/)
{
}

void
ComputeBase::onDomainChanged(short int /*subdomain*/)
{
}

void
ComputeBase::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, short int /*bnd_id*/)
{
}

void
ComputeBase::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}
