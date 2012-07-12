from options import *

num_constants_test = { INPUT : 'num_constants_test.i',
                       EXPECT_ERR : "Exactly two elastic constants must be defined for material 'goo'"}

bulk_modulus_test = { INPUT : 'bulk_modulus_test.i',
                      EXPECT_ERR : "Bulk modulus must be positive in material 'goo'"}

poissons_ratio_test = { INPUT : 'poissons_ratio_test.i',
                        EXPECT_ERR : "Poissons ratio must be greater than -1 and less than 0.5 in material 'goo'"}

shear_modulus_test = { INPUT : 'shear_modulus_test.i',
                       EXPECT_ERR : "Shear modulus must not be negative in material 'goo'"}

youngs_modulus_test = { INPUT : 'youngs_modulus_test.i',
                        EXPECT_ERR : "Youngs modulus must be positive in material 'goo'"}

increment_options_test = { INPUT : 'increment_options_test.i',
                           EXPECT_ERR : "The options for the increment calculation are RashidApprox and Eigen."}

pressure_component_test = { INPUT : 'pressure_component.i',
                            EXPECT_ERR : "Invalid component given for fred: 123456789."}

contact_displacements_test = { INPUT : 'contact_displacements.i',
                               EXPECT_ERR : "Contact requires updated coordinates.  Use the 'displacements"}


