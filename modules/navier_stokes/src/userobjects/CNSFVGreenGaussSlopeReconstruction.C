/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVGreenGaussSlopeReconstruction.h"

template <>
InputParameters
validParams<CNSFVGreenGaussSlopeReconstruction>()
{
  InputParameters params = validParams<SlopeReconstructionMultiD>();

  params.addClassDescription("A user object that performs Green-Gauss slope reconstruction to get "
                             "the slopes of the P0 primitive variables.");

  params.addRequiredCoupledVar("rho", "Density at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhou", "X-momentum at P0 (constant monomial)");

  params.addCoupledVar("rhov", "Y-momentum at P0 (constant monomial)");

  params.addCoupledVar("rhow", "Z-momentum at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhoe", "Total energy at P0 (constant monomial)");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVGreenGaussSlopeReconstruction::CNSFVGreenGaussSlopeReconstruction(
    const InputParameters & parameters)
  : SlopeReconstructionMultiD(parameters),
    _rho(getVar("rho", 0)),
    _rhou(getVar("rhou", 0)),
    _rhov(isCoupled("rhov") ? getVar("rhov", 0) : NULL),
    _rhow(isCoupled("rhow") ? getVar("rhow", 0) : NULL),
    _rhoe(getVar("rhoe", 0)),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
CNSFVGreenGaussSlopeReconstruction::reconstructElementSlope()
{
  const Elem * elem = _current_elem;

  /// current element id
  dof_id_type elemID = elem->id();

  /// number of sides surrounding an element
  unsigned int nside = elem->n_sides();

  /// number of conserved variables
  unsigned int nvars = 5;

  /// vector for the reconstructed gradients of the conserved variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));

  /// local side centroid
  Point scent = Point(0., 0., 0.);

  /// local side normal
  Point snorm = Point(0., 0., 0.);

  /// local side area
  Real sarea = 0.;

  /// local distance between centroids
  Point dcent = Point(0., 0., 0.);

  /// vector for the cell-average values in the central cell and its neighbors
  std::vector<std::vector<Real>> ucell(nside + 1, std::vector<Real>(nvars, 0.));

  /// get average conserved variables in the central cell
  /// and convert them into primitive variables

  Real rhoc = _rho->getElementalValue(elem);
  Real rhomc = 1. / rhoc;
  Real rhouc = _rhou->getElementalValue(elem);
  Real rhovc = 0.;
  Real rhowc = 0.;
  Real rhoec = _rhoe->getElementalValue(elem);
  if (_rhov != NULL)
    rhovc = _rhov->getElementalValue(elem);
  if (_rhow != NULL)
    rhowc = _rhow->getElementalValue(elem);

  ucell[0][1] = rhouc * rhomc;
  ucell[0][2] = rhovc * rhomc;
  ucell[0][3] = rhowc * rhomc;
  Real eintc =
      rhoec * rhomc -
      0.5 * (ucell[0][1] * ucell[0][1] + ucell[0][2] * ucell[0][2] + ucell[0][3] * ucell[0][3]);
  ucell[0][0] = _fp.pressure(rhomc, eintc);
  ucell[0][4] = _fp.temperature(rhomc, eintc);

  /// cache the average variable values of the current element
  _avars[elemID] = ucell[0];

  /// centroid distances of element-side (ES) and neighbor-side (NS)
  Real dES = 0.;
  Real dNS = 0.;
  Real wESN = 0.;

  bool bndElem = false;

  /// loop over the sides to compute the reconstructed slope of this cell

  for (unsigned int is = 0; is < nside; is++)
  {
    unsigned int in = is + 1;

    /// for internal side

    if (elem->neighbor(is) != NULL)
    {
      const Elem * neig = elem->neighbor(is);
      dof_id_type neigID = neig->id();

      if (_side_geoinfo_cached)
      {
        scent = getSideCentroid(elemID, neigID);
        snorm = getSideNormal(elemID, neigID);
        sarea = getSideArea(elemID, neigID);
      }
      else
      {
        _assembly.reinit(elem, is);

        scent = _side_elem->centroid();
        snorm = _normals_face[0];
        sarea = _side_volume;

        _side_centroid[std::pair<dof_id_type, dof_id_type>(elemID, neigID)] = scent;
        _side_normal[std::pair<dof_id_type, dof_id_type>(elemID, neigID)] = snorm;
        _side_area[std::pair<dof_id_type, dof_id_type>(elemID, neigID)] = sarea;
      }

      /// get conserved variables in the current neighbor
      /// and convert them into primitive variables

      Real v = 1. / _rho->getElementalValue(neig);

      ucell[in][1] = _rhou->getElementalValue(neig) * v;
      if (_rhov != NULL)
        ucell[in][2] = _rhov->getElementalValue(neig) * v;
      if (_rhow != NULL)
        ucell[in][3] = _rhow->getElementalValue(neig) * v;

      Real e = _rhoe->getElementalValue(neig) * v -
               0.5 * (ucell[in][1] * ucell[in][1] + ucell[in][2] * ucell[in][2] +
                      ucell[in][3] * ucell[in][3]);

      ucell[in][0] = _fp.pressure(v, e);
      ucell[in][4] = _fp.temperature(v, e);

      /// distance of element center and side center
      dcent = elem->centroid() - scent;
      dES = std::sqrt(dcent * dcent);

      /// distance of neighbor center and side center
      dcent = neig->centroid() - scent;
      dNS = std::sqrt(dcent * dcent);

      /// distance weight
      wESN = dES / (dES + dNS);

      /// cache the average variable values of neighbor element
      _avars[neigID] = ucell[in];
    }

    /// for boundary side

    else
    {
      bndElem = true;

      if (_side_geoinfo_cached)
      {
        scent = getBoundarySideCentroid(elemID, is);
        snorm = getBoundarySideNormal(elemID, is);
        sarea = getBoundarySideArea(elemID, is);
      }
      else
      {
        _assembly.reinit(elem, is);

        scent = _side_elem->centroid();
        snorm = _normals_face[0];
        sarea = _side_volume;

        _bnd_side_centroid[std::pair<dof_id_type, unsigned int>(elemID, is)] = scent;
        _bnd_side_normal[std::pair<dof_id_type, unsigned int>(elemID, is)] = snorm;
        _bnd_side_area[std::pair<dof_id_type, unsigned int>(elemID, is)] = sarea;
      }

      /// get the cell-average values of this ghost cell

      std::vector<BoundaryID> bndID = _mesh.getBoundaryIDs(elem, is);

      for (unsigned int ib = 0; ib < bndID.size(); ib++)
      {
        BoundaryID currentID = bndID[ib];

        std::map<BoundaryID, UserObjectName>::const_iterator pos = _bnd_uo_name_map.find(currentID);

        if (pos != _bnd_uo_name_map.end())
        {
          const BCUserObject & bcuo =
              GeneralUserObject::_fe_problem.getUserObject<BCUserObject>(pos->second);

          std::vector<Real> uvec1 = {rhoc, rhouc, rhovc, rhowc, rhoec};

          ucell[in] = bcuo.getGhostCellValue(is, elem->id(), uvec1, snorm);

          /// convert into primitive variables

          Real rhog = ucell[in][0];
          Real rhoug = ucell[in][1];
          Real rhovg = ucell[in][2];
          Real rhowg = ucell[in][3];
          Real rhoeg = ucell[in][4];

          Real rhomg = 1. / rhog;

          ucell[in][1] = rhoug * rhomg;
          ucell[in][2] = rhovg * rhomg;
          ucell[in][3] = rhowg * rhomg;

          Real eintg = rhoeg * rhomg -
                       0.5 * (ucell[in][1] * ucell[in][1] + ucell[in][2] * ucell[in][2] +
                              ucell[in][3] * ucell[in][3]);

          ucell[in][0] = _fp.pressure(rhomg, eintg);
          ucell[in][4] = _fp.temperature(rhomg, eintg);
        }
      }

      /// distance weight
      wESN = 0.5;

      /// cache the average variable values of ghost element
      _bnd_avars[std::pair<dof_id_type, unsigned int>(elemID, is)] = ucell[in];
    }

    /// sum up the contribution from the current side
    snorm *= sarea / _current_elem_volume;
    for (unsigned int iv = 0; iv < nvars; iv++)
      ugrad[iv] += (wESN * ucell[0][iv] + (1. - wESN) * ucell[in][iv]) * snorm;
  }

  _rslope[elemID] = ugrad;
}
