def alpha(k, rho0, cp):
    return k/(rho0*cp)

def Ra(l, alpha, mu, delta_rho, g):
    return (l**2/alpha)/(mu/(delta_rho*l*g))

l = 300
g = 1
k = 1
cp = 1
rhoh = 1.18
rhol = 1.146
rho0 = (rhoh + rhol) / 2.
delta_rho = rhoh - rhol
alpha_arg = alpha(k, rho0, cp)
mu = 1
print(Ra(l, alpha_arg, mu, delta_rho, g))
