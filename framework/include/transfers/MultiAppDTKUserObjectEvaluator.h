#ifdef LIBMESH_HAVE_DTK

#ifndef MULTIAPPDTKUSEROBJECTEVALUATOR_H
#define MULTIAPPDTKUSEROBJECTEVALUATOR_H

#include "MultiApp.h"

#include <DTK_FieldEvaluator.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_GeometryManager.hpp>
#include <DTK_Box.hpp>

class MultiAppDTKUserObjectEvaluator: public DataTransferKit::FieldEvaluator<
  int, DataTransferKit::FieldContainer<double> >
{
public:
  MultiAppDTKUserObjectEvaluator(MultiApp & multi_app, const std::string & user_object_name);

  ~MultiAppDTKUserObjectEvaluator();

  DataTransferKit::FieldContainer<double> evaluate(const Teuchos::ArrayRCP<int>& bids, const Teuchos::ArrayRCP<double>& coords);

  Teuchos::RCP<DataTransferKit::GeometryManager<DataTransferKit::Box,int> > createSourceGeometry(const Teuchos::RCP<const Teuchos::Comm<int> >& comm);

private:
  /// The MultiAppUserObject object this object will be evaluating
  MultiApp & _multi_app;

  /// The name of the UserObject we're going to be evaluating
  std::string _user_object_name;

  Teuchos::ArrayRCP<DataTransferKit::Box> _boxes;
  Teuchos::ArrayRCP<int> _box_ids;
};


#endif //MULTIAPPDTKUSEROBJECTEVALUATOR_H

#endif //LIBMESH_HAVE_DTK
