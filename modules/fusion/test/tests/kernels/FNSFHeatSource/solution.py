import mms

fs,ss =mms.evaluate('-div(k * grad(T)) - S', 
			'sin(pi*x)*cos(pi*y)*sin(pi*z)',
			variable='T', scalars=['k', 'S'])
mms.print_hit(fs, 'mms_force')
mms.print_hit(ss, 'mms_exact') 
