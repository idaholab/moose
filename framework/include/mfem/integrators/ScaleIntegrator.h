#pragma once
#include "../common/pfem_extras.hpp"

using namespace mfem;

/// Integrator which scales its results by a constant value
class ScaleIntegrator : public BilinearFormIntegrator
{
private:
   bool _own_integrator;
   double _scale;
   BilinearFormIntegrator* _integrator{nullptr};

public:
   
   ScaleIntegrator(BilinearFormIntegrator *integ) : _integrator{integ}, _scale{1}, _own_integrator{true} {}
   ScaleIntegrator(BilinearFormIntegrator *integ, double scale) : _integrator{integ}, _scale{scale}, _own_integrator{true} {}
   ScaleIntegrator(BilinearFormIntegrator *integ, double scale, bool own) : _integrator{integ}, _scale{scale}, _own_integrator{own} {}

   void SetIntegrator(BilinearFormIntegrator *integ)
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

   virtual void SetIntRule(const IntegrationRule *ir);


   virtual void AssembleElementMatrix(const FiniteElement &el,
                                      ElementTransformation &Trans,
                                      DenseMatrix &elmat);
   virtual void AssembleElementMatrix2(const FiniteElement &trial_fe,
                                       const FiniteElement &test_fe,
                                       ElementTransformation &Trans,
                                       DenseMatrix &elmat);

   using BilinearFormIntegrator::AssembleFaceMatrix;
   virtual void AssembleFaceMatrix(const FiniteElement &el1,
                                   const FiniteElement &el2,
                                   FaceElementTransformations &Trans,
                                   DenseMatrix &elmat);

   virtual void AssembleFaceMatrix(const FiniteElement &trial_face_fe,
                                   const FiniteElement &test_fe1,
                                   const FiniteElement &test_fe2,
                                   FaceElementTransformations &Trans,
                                   DenseMatrix &elmat);

   using BilinearFormIntegrator::AssemblePA;
   virtual void AssemblePA(const FiniteElementSpace& fes);

   virtual void AssembleDiagonalPA(Vector &diag);

   virtual void AssemblePAInteriorFaces(const FiniteElementSpace &fes);

   virtual void AssemblePABoundaryFaces(const FiniteElementSpace &fes);

   virtual void AddMultTransposePA(const Vector &x, Vector &y) const;

   virtual void AddMultPA(const Vector& x, Vector& y) const;

   virtual void AssembleMF(const FiniteElementSpace &fes);

   virtual void AddMultMF(const Vector &x, Vector &y) const;

   virtual void AddMultTransposeMF(const Vector &x, Vector &y) const;

   virtual void AssembleDiagonalMF(Vector &diag);

   virtual void AssembleEA(const FiniteElementSpace &fes, Vector &emat,
                           const bool add);

   virtual void AssembleEAInteriorFaces(const FiniteElementSpace &fes,
                                        Vector &ea_data_int,
                                        Vector &ea_data_ext,
                                        const bool add);

   virtual void AssembleEABoundaryFaces(const FiniteElementSpace &fes,
                                        Vector &ea_data_bdr,
                                        const bool add);

   virtual ~ScaleIntegrator();
};
