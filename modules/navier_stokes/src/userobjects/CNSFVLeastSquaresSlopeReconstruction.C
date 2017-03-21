/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVLeastSquaresSlopeReconstruction.h"

template <>
InputParameters
validParams<CNSFVLeastSquaresSlopeReconstruction>()
{
  InputParameters params = validParams<SlopeReconstructionMultiD>();

  params.addClassDescription("A user object that performs the least-squares slope reconstruction "
                             "to get the slopes of the P0 primitive variables.");

  params.addRequiredCoupledVar("rho", "Density at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhou", "X-momentum at P0 (constant monomial)");

  params.addCoupledVar("rhov", "Y-momentum at P0 (constant monomial)");

  params.addCoupledVar("rhow", "Z-momentum at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhoe", "Total energy at P0 (constant monomial)");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVLeastSquaresSlopeReconstruction::CNSFVLeastSquaresSlopeReconstruction(
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
CNSFVLeastSquaresSlopeReconstruction::reconstructElementSlope()
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

  /// local side normal
  Point snorm = Point(0., 0., 0.);

  /// local side centroid
  Point scent = Point(0., 0., 0.);

  /// local distance between centroids
  Point dcent = Point(0., 0., 0.);

  /// vector for the cell-average values in the central cell and its neighbors
  std::vector<std::vector<Real>> u(nside + 1, std::vector<Real>(nvars, 0.));

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

  u[0][1] = rhouc * rhomc;
  u[0][2] = rhovc * rhomc;
  u[0][3] = rhowc * rhomc;
  Real eintc = rhoec * rhomc - 0.5 * (u[0][1] * u[0][1] + u[0][2] * u[0][2] + u[0][3] * u[0][3]);
  u[0][0] = _fp.pressure(rhomc, eintc);
  u[0][4] = _fp.temperature(rhomc, eintc);

  /// cache the average variable values of the current element
  _avars[elemID] = u[0];

  /// LHS matrix components
  Real A11 = 0., A12 = 0., A13 = 0., A22 = 0., A23 = 0., A33 = 0.;

  /// inverse of LHS matrix compoenents
  Real C11 = 0., C12 = 0., C13 = 0., C21 = 0., C22 = 0., C23 = 0., C31 = 0., C32 = 0., C33 = 0.;

  /// RHS vector components
  std::vector<Real> B1(nvars, 0.);
  std::vector<Real> B2(nvars, 0.);
  std::vector<Real> B3(nvars, 0.);

  /// coefficients
  Real lspow = 2., lswei = 0.;

  /// helpful scalars
  Real invdet = 0., dx = 0., dy = 0., dz = 0.;

  /// helpful vectors
  std::vector<Real> du(nvars, 0.);

  /// a flag to indicate boundary element
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

      if (!_side_geoinfo_cached)
      {
        _assembly.reinit(elem, is);
        _side_centroid[std::pair<dof_id_type, dof_id_type>(elemID, neigID)] =
            _side_elem->centroid();
      }

      /// get conserved variables in the current neighbor
      /// and convert them into primitive variables

      Real v = 1. / _rho->getElementalValue(neig);

      u[in][1] = _rhou->getElementalValue(neig) * v;
      if (_rhov != NULL)
        u[in][2] = _rhov->getElementalValue(neig) * v;
      if (_rhow != NULL)
        u[in][3] = _rhow->getElementalValue(neig) * v;

      Real e = _rhoe->getElementalValue(neig) * v -
               0.5 * (u[in][1] * u[in][1] + u[in][2] * u[in][2] + u[in][3] * u[in][3]);

      u[in][0] = _fp.pressure(v, e);
      u[in][4] = _fp.temperature(v, e);

      /// cache the average variable values of neighbor element
      _avars[neigID] = u[in];

      /// form the matrix-vector components

      dcent = neig->centroid() - elem->centroid();

      lswei = std::sqrt(dcent * dcent);

      lswei = std::pow(lswei, -lspow);

      for (unsigned int iv = 0; iv < nvars; iv++)
        du[iv] = u[in][iv] - u[0][iv];

      if (_mesh.dimension() > 0)
      {
        dx = dcent(0);
        A11 += lswei * dx * dx;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B1[iv] += lswei * du[iv] * dx;
      }
      if (_mesh.dimension() > 1)
      {
        dy = dcent(1);
        A12 += lswei * dx * dy;
        A22 += lswei * dy * dy;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B2[iv] += lswei * du[iv] * dy;
      }
      if (_mesh.dimension() > 2)
      {
        dz = dcent(2);
        A13 += lswei * dx * dz;
        A23 += lswei * dy * dz;
        A33 += lswei * dz * dz;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B3[iv] += lswei * du[iv] * dz;
      }
    }

    /// for boundary side

    else
    {
      bndElem = true;

      if (_side_geoinfo_cached)
      {
        scent = getBoundarySideCentroid(elemID, is);
        snorm = getBoundarySideNormal(elemID, is);
      }
      else
      {
        _assembly.reinit(elem, is);
        scent = _side_elem->centroid();
        snorm = _normals_face[0];
        _bnd_side_centroid[std::pair<dof_id_type, unsigned int>(elemID, is)] = scent;
        _bnd_side_normal[std::pair<dof_id_type, unsigned int>(elemID, is)] = snorm;
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

          // another question is if we can use (rho,u,v,w,p) as primitive variabls
          // instead of the current (p,u,v,w,T)
          // TODO test it in 1D problems first

          std::vector<Real> uvec1 = {rhoc, rhouc, rhovc, rhowc, rhoec};

          u[in] = bcuo.getGhostCellValue(is, elem->id(), uvec1, snorm);

          /// convert into primitive variables

          Real rhog = u[in][0];
          Real rhoug = u[in][1];
          Real rhovg = u[in][2];
          Real rhowg = u[in][3];
          Real rhoeg = u[in][4];

          Real rhomg = 1. / rhog;

          u[in][1] = rhoug * rhomg;
          u[in][2] = rhovg * rhomg;
          u[in][3] = rhowg * rhomg;

          Real eintg = rhoeg * rhomg -
                       0.5 * (u[in][1] * u[in][1] + u[in][2] * u[in][2] + u[in][3] * u[in][3]);

          u[in][0] = _fp.pressure(rhomg, eintg);
          u[in][4] = _fp.temperature(rhomg, eintg);
        }
      }

      /// cache the average variable values of ghost element
      _bnd_avars[std::pair<dof_id_type, unsigned int>(elemID, is)] = u[in];

      /// form the matrix-vector components

      dcent = 2. * (scent - elem->centroid());

      lswei = std::sqrt(dcent * dcent);

      lswei = std::pow(lswei, -lspow);

      for (unsigned int iv = 0; iv < nvars; iv++)
        du[iv] = u[in][iv] - u[0][iv];

      if (_mesh.dimension() > 0)
      {
        dx = dcent(0);
        A11 += lswei * dx * dx;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B1[iv] += lswei * du[iv] * dx;
      }
      if (_mesh.dimension() > 1)
      {
        dy = dcent(1);
        A12 += lswei * dx * dy;
        A22 += lswei * dy * dy;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B2[iv] += lswei * du[iv] * dy;
      }
      if (_mesh.dimension() > 2)
      {
        dz = dcent(2);
        A13 += lswei * dx * dz;
        A23 += lswei * dy * dz;
        A33 += lswei * dz * dz;
        for (unsigned int iv = 0; iv < nvars; iv++)
          B3[iv] += lswei * du[iv] * dz;
      }
    }
  }

  /// solve Ax=B using the normal equation approach

  if (_mesh.dimension() == 1)
  {
    for (unsigned int iv = 0; iv < nvars; iv++)
      ugrad[iv](0) = B1[iv] / A11;
  }
  else if (_mesh.dimension() == 2)
  {
    invdet = 1. / (A11 * A22 - A12 * A12);

    for (unsigned int iv = 0; iv < nvars; iv++)
    {
      ugrad[iv](0) = invdet * (A22 * B1[iv] - A12 * B2[iv]);
      ugrad[iv](1) = invdet * (-A12 * B1[iv] + A11 * B2[iv]);
    }
  }
  else if (_mesh.dimension() == 3)
  {
    C11 = A22 * A33 - A23 * A23;
    C12 = A13 * A23 - A12 * A33;
    C13 = A12 * A23 - A13 * A22;

    C21 = A13 * A23 - A12 * A33;
    C22 = A11 * A33 - A13 * A13;
    C23 = A12 * A13 - A11 * A23;

    C31 = A12 * A23 - A22 * A13;
    C32 = A12 * A13 - A11 * A23;
    C33 = A11 * A22 - A12 * A12;

    invdet = 1. / (A11 * C11 + A12 * C12 + A13 * C12);

    for (unsigned int iv = 0; iv < nvars; iv++)
    {
      ugrad[iv](0) = invdet * (C11 * B1[iv] + C12 * B2[iv] + C13 * B3[iv]);
      ugrad[iv](1) = invdet * (C21 * B1[iv] + C22 * B2[iv] + C23 * B3[iv]);
      ugrad[iv](2) = invdet * (C31 * B1[iv] + C32 * B2[iv] + C33 * B3[iv]);
    }
  }

  _rslope[elemID] = ugrad;
}
