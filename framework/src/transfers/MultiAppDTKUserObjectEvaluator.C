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

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "MultiAppDTKUserObjectEvaluator.h"

// Moose
#include "UserObject.h"
#include "Executioner.h"
#include "FEProblem.h"

#include <Teuchos_DefaultComm.hpp>

#include "MooseTypes.h"

MultiAppDTKUserObjectEvaluator::MultiAppDTKUserObjectEvaluator(MultiApp & multi_app, const std::string & user_object_name):
    _multi_app(multi_app),
    _user_object_name(user_object_name)
{
}

MultiAppDTKUserObjectEvaluator::~MultiAppDTKUserObjectEvaluator()
{
}

DataTransferKit::FieldContainer<double>
MultiAppDTKUserObjectEvaluator::evaluate(const Teuchos::ArrayRCP<int>& bids, const Teuchos::ArrayRCP<double>& coords)
{
  Teuchos::RCP<const Teuchos::Comm<int> > comm = Teuchos::DefaultComm<int>::getComm();

  int num_values = bids.size();

  Teuchos::ArrayRCP<double> evaluated_data(num_values);

  unsigned int dim = 3;  // TODO: REPLACE ME!!!!!!!!!

  for (int i=0; i<num_values; i++)
  {
    // See if this app is on this processor
    if(std::binary_search(_box_ids.begin(), _box_ids.end(), bids[i]))
    {
      unsigned int app = bids[i];

      Point p;
      for(unsigned int j=0; j<dim; j++)
        p(j) = coords[(j*num_values)+i];

      evaluated_data[i] = _multi_app.appUserObjectBase(app, _user_object_name).spatialValue(p);
    }
    else
      evaluated_data[i] = 0.0;
  }

  return DataTransferKit::FieldContainer<double>(evaluated_data, 1);
}

Teuchos::RCP<DataTransferKit::GeometryManager<DataTransferKit::Box,int> >
MultiAppDTKUserObjectEvaluator::createSourceGeometry( const Teuchos::RCP<const Teuchos::Comm<int> >& comm )
{
  _boxes.resize(_multi_app.numLocalApps());
  _box_ids.resize(_multi_app.numLocalApps());

  // How much to inflate each bounding box by (helps with non-matching surfaces)
  unsigned int inflation = 0.01;

  comm->barrier();

  for(unsigned int app=0; app<_multi_app.numLocalApps(); app++)
  {
    unsigned int global_app = _multi_app.firstLocalApp() + app;

    MeshTools::BoundingBox bbox = _multi_app.getBoundingBox(global_app);

    _boxes[app] = DataTransferKit::Box(bbox.min()(0), bbox.min()(1), bbox.min()(2),
                                       bbox.max()(0), bbox.max()(1), bbox.max()(2));

    _box_ids[app] = global_app;
  }

  return Teuchos::rcp( new DataTransferKit::GeometryManager<DataTransferKit::Box,int>(_boxes, _box_ids, comm, 3));

}

#endif //LIBMESH_HAVE_DTK
