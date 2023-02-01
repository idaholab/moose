#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
import unittest
import platform
import mooseutils
import mooseutils.civet_results as cr

SITE = 'https://mooseframework.inl.gov/docs/civettest'
REPO = 'idaholab/moose'
SHAS = ['681ba2f4274dc8465bb2a54e1353cfa24765a5c1', 'febe3476040fe6af1df1d67e8cc8c04c4760afb6']


class Test(unittest.TestCase):
    def testGetCivetJobs(self):
        jobs = cr._get_remote_civet_jobs(SHAS, SITE, REPO)
        self.assertEqual(len(jobs), 67)
        self.assertEqual(jobs[0].number, 443457)
        self.assertTrue(jobs[0].filename.endswith('results_443457.tar.gz'))
        self.assertEqual(jobs[0].url, SITE)

    def testUpdateDatabaseFromJob(self):
        jobs = cr._get_remote_civet_jobs(SHAS, SITE, REPO)
        database = collections.defaultdict(lambda: collections.defaultdict(list))
        cr._update_database_from_job(jobs[42], database, None)

        tests = database['kernels/simple_diffusion.test'][443499]
        self.assertEqual(tests[1].recipe, '06_Test_-p_3')
        self.assertEqual(tests[1].status, 'OK')
        self.assertEqual(tests[1].caveats, ['recover'])
        self.assertEqual(tests[1].url, SITE)
        self.assertEqual(tests[1].reason, '')

    def testGetCivetResults(self):
        database = cr.get_civet_results(hashes=SHAS, site=(SITE, REPO))
        tests = database['kernels/simple_diffusion.test'][443499]
        self.assertEqual(tests[1].recipe, '06_Test_-p_3')
        self.assertEqual(tests[1].status, 'OK')
        self.assertEqual(tests[1].caveats, ['recover'])
        self.assertEqual(tests[1].url, SITE)
        self.assertEqual(tests[1].reason, '')

    @unittest.skipIf(platform.python_version() < '3.7.0', "Python 3.7 or greater required.")
    def testGetCivetHashes(self):

        # Release 2021-05-18
        gold = ('90123e7b6bd52f1bc36e68aac5d1fa95e76aeb91', 'd72a8d0d69e21b4945eedf2e78a7de80b1bd3e6f')
        hashes = mooseutils.get_civet_hashes('2021-05-18')
        self.assertEqual(hashes, gold)

        gold = ('df827bfaf6ea29394ce609bdf032bd40a9818cfc', 'c4ec8d4669166086da10470cc99c4b40813eeee9')
        hashes = mooseutils.get_civet_hashes('df827bfaf6ea29394ce609bdf032bd40a9818cfc')
        self.assertEqual(hashes, gold)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
