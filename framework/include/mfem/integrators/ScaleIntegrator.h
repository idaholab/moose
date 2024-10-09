#pragma once
#include "MFEMGeneralUserObject.h"
#include "../common/pfem_extras.hpp"

/// Integrator which scales its results by a constant value
class ScaleIntegrator : public mfem::BilinearFormIntegrator
{
private:

  mfem::BilinearFormIntegrator* _integrator{nullptr};
  double _scale;
  bool _own_integrator;  

public:
   
   ScaleIntegrator(mfem::BilinearFormIntegrator *integ) : _integrator{integ}, _scale{1}, _own_integrator{true} {}
   ScaleIntegrator(mfem::BilinearFormIntegrator *integ, double scale) : _integrator{integ}, _scale{scale}, _own_integrator{true} {}
   ScaleIntegrator(mfem::BilinearFormIntegrator *integ, double scale, bool own) : _integrator{integ}, _scale{scale}, _own_integrator{own} {}

   void SetIntegrator(mfem::BilinearFormIntegrator *integ)
   { 
      if (_integrator && _own_integrator)
      {
          delete _integrator;
      }

      _integrator = integ; 
    }

   void SetScale(double scale) {_scale = scale;}

   void SetOwn(bool own) {_own_integrator = own;}

   void CheckIntegrator()
   {
      if (!_integrator)
        mooseError("Integrator not set");
   }

   virtual void SetIntRule(const mfem::IntegrationRule *ir);


   virtual void AssembleElementMatrix(const mfem::FiniteElement &el,
                                      mfem::ElementTransformation &Trans,
                                      mfem::DenseMatrix &elmat);
   virtual void AssembleElementMatrix2(const mfem::FiniteElement &trial_fe,
                                       const mfem::FiniteElement &test_fe,
                                       mfem::ElementTransformation &Trans,
                                       mfem::DenseMatrix &elmat);

   using mfem::BilinearFormIntegrator::AssembleFaceMatrix;
   virtual void AssembleFaceMatrix(const mfem::FiniteElement &el1,
                                   const mfem::FiniteElement &el2,
                                   mfem::FaceElementTransformations &Trans,
                                   mfem::DenseMatrix &elmat);

   virtual void AssembleFaceMatrix(const mfem::FiniteElement &trial_face_fe,
                                   const mfem::FiniteElement &test_fe1,
                                   const mfem::FiniteElement &test_fe2,
                                   mfem::FaceElementTransformations &Trans,
                                   mfem::DenseMatrix &elmat);

   using mfem::BilinearFormIntegrator::AssemblePA;
   virtual void AssemblePA(const mfem::FiniteElementSpace& fes);

   virtual void AssembleDiagonalPA(mfem::Vector &diag);

   virtual void AssemblePAInteriorFaces(const mfem::FiniteElementSpace &fes);

   virtual void AssemblePABoundaryFaces(const mfem::FiniteElementSpace &fes);

   virtual void AddMultTransposePA(const mfem::Vector &x, mfem::Vector &y) const;

   virtual void AddMultPA(const mfem::Vector& x, mfem::Vector& y) const;

   virtual void AssembleMF(const mfem::FiniteElementSpace &fes);

   virtual void AddMultMF(const mfem::Vector &x, mfem::Vector &y) const;

   virtual void AddMultTransposeMF(const mfem::Vector &x, mfem::Vector &y) const;

   virtual void AssembleDiagonalMF(mfem::Vector &diag);

   virtual void AssembleEA(const mfem::FiniteElementSpace &fes, mfem::Vector &emat,
                           const bool add);

   virtual void AssembleEAInteriorFaces(const mfem::FiniteElementSpace &fes,
                                        mfem::Vector &ea_data_int,
                                        mfem::Vector &ea_data_ext,
                                        const bool add);

   virtual void AssembleEABoundaryFaces(const mfem::FiniteElementSpace &fes,
                                        mfem::Vector &ea_data_bdr,
                                        const bool add);

   virtual ~ScaleIntegrator();
};
