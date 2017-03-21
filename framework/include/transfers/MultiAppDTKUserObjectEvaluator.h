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

#ifndef MULTIAPPDTKUSEROBJECTEVALUATOR_H
#define MULTIAPPDTKUSEROBJECTEVALUATOR_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

// Forward declarations
class MultiApp;

// DTK includes
#include "libmesh/ignore_warnings.h"
#include <DTK_FieldEvaluator.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_GeometryManager.hpp>
#include <DTK_Box.hpp>
#include "libmesh/restore_warnings.h"

/**
 * Evaluates the specified UserObject and returns the result in a DTK FieldContainer.
 */
class MultiAppDTKUserObjectEvaluator
    : public DataTransferKit::FieldEvaluator<long unsigned int,
                                             DataTransferKit::FieldContainer<double>>
{
public:
  MultiAppDTKUserObjectEvaluator(MultiApp & multi_app, const std::string & user_object_name);

  ~MultiAppDTKUserObjectEvaluator();

  typedef long unsigned int GlobalOrdinal;

  DataTransferKit::FieldContainer<double> evaluate(const Teuchos::ArrayRCP<GlobalOrdinal> & bids,
                                                   const Teuchos::ArrayRCP<double> & coords);

  Teuchos::RCP<DataTransferKit::GeometryManager<DataTransferKit::Box, GlobalOrdinal>>
  createSourceGeometry(const Teuchos::RCP<const Teuchos::Comm<int>> & comm);

private:
  /// The MultiAppUserObject object this object will be evaluating
  MultiApp & _multi_app;

  /// The name of the UserObject we're going to be evaluating
  std::string _user_object_name;

  Teuchos::ArrayRCP<DataTransferKit::Box> _boxes;
  Teuchos::ArrayRCP<GlobalOrdinal> _box_ids;
};

#endif // LIBMESH_TRILINOS_HAVE_DTK
#endif // MULTIAPPDTKUSEROBJECTEVALUATOR_H
