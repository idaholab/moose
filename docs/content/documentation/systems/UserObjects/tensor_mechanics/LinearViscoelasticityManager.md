# LinearViscoelasticityManager
This updates the internal time-stepping scheme for linear viscoelastic materials (such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material) at the beginning of each time step. Including a LinearViscoelasticStressUpdate in the simulation is required for the linear viscoelastic strains to be updated properly.

!syntax description /UserObjects/LinearViscoelasticityManager

!syntax parameters /UserObjects/LinearViscoelasticityManager

!syntax inputs /UserObjects/LinearViscoelasticityManager

!syntax children /UserObjects/LinearViscoelasticityManager
