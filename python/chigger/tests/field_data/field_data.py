#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
reader.update()
print reader.getGlobalData('k_eff')
