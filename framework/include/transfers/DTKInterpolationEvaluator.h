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
#ifndef DTKINTERPOLATIONEVALUATOR_H
#define DTKINTERPOLATIONEVALUATOR_H


#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "libmesh/equation_systems.h"
#include "libmesh/mesh.h"
#include "libmesh/system.h"

#include <DTK_MeshContainer.hpp>
#include <DTK_FieldEvaluator.hpp>
#include <DTK_FieldContainer.hpp>

#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>

#include <string>

namespace libMesh {

class DTKInterpolationEvaluator : public DataTransferKit::FieldEvaluator<long unsigned int,DataTransferKit::FieldContainer<double> >
{
public:
  typedef DataTransferKit::MeshContainer<long unsigned int>        MeshContainerType;
  typedef DataTransferKit::FieldContainer<Number>     FieldContainerType;
  typedef DataTransferKit::MeshTraits<MeshContainerType>::global_ordinal_type  GlobalOrdinal;

  DTKInterpolationEvaluator(System & in_sys, std::string var_name, const Point & offset);

  FieldContainerType evaluate( const Teuchos::ArrayRCP<GlobalOrdinal>& elements,
                               const Teuchos::ArrayRCP<double>& coords );

protected:
  System & sys;
  Point _offset;
  NumericVector<Number> & current_local_solution;
  EquationSystems & es;
  MeshBase & mesh;
  unsigned int dim;
  DofMap & dof_map;
  unsigned int var_num;
  const FEType& fe_type;
};

} // namespace libMesh

#endif // #ifdef LIBMESH_HAVE_DTK

#endif // #define DTKINTERPOLATIONEVALUATOR_H
