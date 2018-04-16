The [ConservedLangevinNoise](/ConservedLangevinNoise.md) kernel uses a plugable user object
that acts as the supplier for the random field. This system allows the generation of
concentration conserving noise. This means the integral over the applied noise field
is guaranteed to be zero. In phase field simulation this avoids a mass-conservation
violating concentration drift. This kernel yields parallel reproducible results.

The available user objects that provide the noise field are

- [ConservedUniformNoise](/ConservedUniformNoise.md)
- [ConservedMaskedUniformNoise](/ConservedMaskedUniformNoise.md)
- [ConservedNormalNoise](/ConservedNormalNoise.md)
- [ConservedMaskedNormalNoise](/ConservedMaskedNormalNoise.md)

Here the *Uniform* stands for a uniform random number distribution, and the *Normal*
stands for a Gaussian normal distribution. User objects with *Masked* in the name
take a material property that is used as a spatially dependent multiplier for the noise
field. This allows selective application of the noise in subregions of the sample
(souch as grain boundaries) or a temperature dependent noise amplitude.
