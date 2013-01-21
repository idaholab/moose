from options import *

test = { INPUT : 'internal_volume.i',
         CSVDIFF : ['out.csv'],
         TYPE : 'CSVDiff' }

test_hex20 = { INPUT : 'internal_volume_hex20.i',
               PREREQ : ['test'],
               CSVDIFF : ['out.csv'],
               TYPE : 'CSVDiff' }

test_rz = { INPUT : 'internal_volume_rz.i',
            CSVDIFF : ['out_rz.csv'],
            TYPE : 'CSVDiff' }

test_quad8_rz = { INPUT : 'internal_volume_rz_quad8.i',
                  PREREQ : ['test_rz'],
                  CSVDIFF : ['out_rz.csv'],
                  TYPE : 'CSVDiff' }

test_rz_displaced = { INPUT : 'internal_volume_rz_displaced.i',
                      CSVDIFF : ['internal_volume_rz_displaced_out.csv'],
                      TYPE : 'CSVDiff' }

test_rz_cone = { INPUT : 'internal_volume_rz_cone.i',
                 CSVDIFF : ['internal_volume_rz_cone_out.csv'],
                 TYPE : 'CSVDiff' }

test_rspherical = { INPUT : 'internal_volume_rspherical.i',
                    CSVDIFF : ['internal_volume_rspherical_out.csv'],
                    TYPE : 'CSVDiff' }

