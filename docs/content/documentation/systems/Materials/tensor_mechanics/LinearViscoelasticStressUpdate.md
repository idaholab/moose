# LinearViscoelasticStressUpdate
This computes the inelastic strain increment resulting from a linear viscoelastic material such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material. It uses an incremental strain approximation (either incremental small strains, or finite strains), and needs to be used in conjunction with [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) or a similar stress calculator.

!syntax description /Materials/LinearViscoelasticStressUpdate

!syntax parameters /Materials/LinearViscoelasticStressUpdate

!syntax inputs /Materials/LinearViscoelasticStressUpdate

!syntax children /Materials/LinearViscoelasticStressUpdate
