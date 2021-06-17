# Draws Pc curves and relperm curves

import os, sys
import math
import matplotlib.pyplot as plt
plt.rcParams.update({'figure.max_open_warning': 0})

def pc_fcn(sl, slmin, sgrdel, alpha, n, sm, pcm, pcmprime, lower_extension_type, sm_upper, pcm_upper, pcmprime_upper, upper_extension_type):
    # Eqn(1) of Doughty2007
    # with extensions defined in Doughty2008, page5 and Fig1
    # lower extension strategy:
    # User inputs Pcmax, which is a value of Pc on the main drainage curve.
    # From this, sm is worked out.  For sl < sm, an extension is used for the main drainage curve
    # Clearly, sm > slmin because to get sm=slmin, absPcmax = infinity would have to be used
    # Similarly for the main wetting curve: Pcmax gives value of sm
    # Upper extension strategy:
    # User inputs r < 1.0 (usually r ~ 1.0), and code calculates sm_upper = r * (1 - sgrdel).
    # For sl > sm_upper, the upper extension is used.  This is a power law that is zero at sl = 1, and matches the main curve's value (pcm_upper) and derivative (pcmprime_upper) at sm_upper.
    if (sl < sm): # important for initialising pcm and pcmprime that this is <, not <=
        if lower_extension_type == "quadratic":
            return pcm + pcmprime * (sl * sl - sm * sm) / 2.0 / sm
        elif lower_extension_type == "exponential":
            return pcm * math.exp(pcmprime * (sl - sm) / pcm)
        else:
            return pcm
    if (sl > sm_upper): # important for initialising pcm_upper and pcmprime_upper that this is >, not >=
        if upper_extension_type == "power":
            expon = -pcmprime_upper / pcm_upper * (1.0 - sm_upper)
            if sl >= 1.0:
                return 0.0
            return pcm_upper * math.pow((1.0 - sl) / (1.0 - sm_upper), expon)
    seff = (sl - slmin) / (1 - sgrdel - slmin)
    if (seff >= 1.0):
        return 0.0 # no valid upper-extension_type defined
    elif (seff <= 0.0):
        return 100.0 # no valid lower_extension_type defined
    return -(1.0 / alpha) * pow(pow(seff, n / (1.0 - n)) - 1.0, 1.0 / n)

def pc_fcn_prime(sl, slmin, sgrdel, alpha, n, sm, pcm, pcmprime, lower_extension_type, sm_upper, pcm_upper, pcmprime_upper, upper_extension_type):
    # derivative of pc_fcn wrt sl
    if (sl < sm): # important for initialising pcm and pcmprime that this is <, not <=
        if lower_extension_type == "quadratic":
            return pcmprime * sl / sm
        elif lower_extension_type == "exponential":
            return pcmprime * math.exp(pcmprime * (sl - sm) / pcm)
    if (sl > sm_upper): # important for initialising pcm_upper and pcmprime_upper that this is >, not >=
        if upper_extension_type == "power":
            return pcmprime_upper * math.pow((1.0 - sl) / (1.0 - sm_upper), -pcmprime_upper / pcm_upper * (1.0 - sm_upper) - 1.0)
    seff = (sl - slmin) / (1.0 - sgrdel - slmin)
    if (seff >= 1.0):
        return 0.0 # no valid upper-extension_type defined
    elif (seff <= 0.0):
        return 0.0 # no valid lower_extension_type defined
    dseff = 1.0 / (1.0 - sgrdel - slmin)
    dpc_dseff = -(1.0 / alpha) * pow(pow(seff, n / (1.0 - n)) - 1.0, 1.0 / n - 1.0) * pow(seff, n / (1.0 - n) - 1.0) / (1.0 - n)
    return dpc_dseff * dseff

def sat_fcn(pc, slmin, sgrdel, alpha, n, sm, pcm, pcmprime, lower_extension_type, sm_upper, pcm_upper, pcmprime_upper, upper_extension_type):
    # inverse of pc_fcn
    if (pc >= 0):
        return 1.0
    if (pc < pcm): # Important for initialisation that the condition is < not <=.  Remember that pc < 0
        if lower_extension_type == "quadratic":
            s2 = sm * sm + (pc - pcm) * 2.0 * sm / pcmprime
            if s2 <= 0.0: # this occurs when we're tyring to find the saturation on the wetting curve defined by self.gr_Del at pc = self.Pcd_Del, if this pc is actually impossible to achieve on this wetting curve
                return 0.0
            return math.sqrt(s2)
        elif lower_extension_type == "exponential":
            s = sm + math.log(pc / pcm) * pcm / pcmprime
            if s < 0.0:  # this occurs when we're tyring to find the saturation on the wetting curve defined by self.gr_Del at pc = self.Pcd_Del, if this pc is actually impossible to achieve on this wetting curve
                return 0.0
            return s
        else:
            return sm
    if (pc > pcm_upper): # Important for initialisation that the condition is > not >=.  Remember that pc < 0
        if upper_extension_type == "power":
            expon = -pcmprime_upper / pcm_upper * (1.0 - sm_upper)
            return 1.0 - math.power(pc / pcm_upper, 1.0 / expon) * (1.0 - sm_upper)
    seffpow = 1.0 + pow(-alpha * pc, n)
    seff = pow(seffpow, (1.0 - n) / n)
    return (1.0 - sgrdel - slmin) * seff + slmin

def cubic(x, x0, y0, y0p, x1, y1, y1p):
    ''' Cubic f(x) that satisfies
    f(x0) = y0
    f'(x0) = y0p
    f(x1) = y1
    f'(x1) = y1p
    '''
    d = x1 - x0
    d2 = math.pow(d, 2.0)
    mean = 0.5 * (x1 + x0)
    sq3 = 0.5 * math.sqrt(3.0) * d
    term1 = y0p * (x - x0) * math.pow(x - x1, 2) / d2 # term1(x0) = term1(x1) = term1'(x1) = 0, term1'(x0) = y0p
    term2 = y1p * (x - x1) * math.pow(x - x0, 2) / d2 # term2(x0) = term2(x1) = term2'(x0) = 0, term2'(x1) = y1p
    term3 = (x - mean - sq3) * (x - mean) * (x - mean + sq3)
    # term3' = (x - mean) * (x - mean + sq3) + (x - mean - sq3) * (x - mean + sq3) + (x - mean - sq3) * (x - mean)
    #        = 3 (x - mean)^2 - sq3^2
    # note term3' = 0 when x = mean +/- sq3/sqrt(3) = 0.5 * (x1 + x0) +/- 0.5 * (x1 - x0) = {x1, x0}
    term3_x0 = (x0 - mean - sq3) * (x0 - mean) * (x0 - mean + sq3)
    term3_x1 = (x1 - mean - sq3) * (x1 - mean) * (x1 - mean + sq3)
    return (y0 * (term3 - term3_x1) + y1 * (term3_x0 - term3)) / (term3_x0 - term3_x1) + term1 + term2

def krel_liquid(sl, slr, sgrdel, sgrmax, sldel, m, upper_liquid_param, y0, y0p, y1, y1p):
    if (sl < slr):
        return 0.0
    sl_bar = (sl - slr) / (1.0 - slr)
    if (sgrdel == 0.0 or sl <= sldel): # along the drying curve
        s_gt_bar = 0.0
        a = math.pow(1.0 - math.pow(sl_bar, 1.0 / m), m)
        b = 0.0
    else:
        if (sldel < slr): # turning point occured below slr, but "there is no hysteresis along the extension" according to p6 of Doughty2008.  The above checked that sl >= slr.  I assume the wetting relperm curve is the same as if the turning point = slr:
            my_sldel = slr
            my_sgrdel = sgrmax
        else:
            my_sldel = sldel
            my_sgrdel = sgrdel
        # This is Doughty2008: if sl >= (2.0 - upper_liquid_param) * (1.0 - my_sgrdel): # imporant for initialisation of cubic that this is >= and not >
        if sl >= 1.0 - 0.5 * my_sgrdel: # imporant for initialisation of cubic that this is >= and not >
            # follow the drying curve
            s_gt_bar = 0.0
            a = math.pow(1.0 - math.pow(sl_bar, 1.0 / m), m)
            b = 0.0
        elif sl > upper_liquid_param * (1.0 - my_sgrdel): # important for initialisation of cubic that this is > and not >=
            # this is Doughty2008: return cubic(sl, upper_liquid_param * (1.0 - my_sgrdel), y0, y0p, (2.0 - upper_liquid_param) * (1.0 - my_sgrdel), y1, y1p)
            return cubic(sl, upper_liquid_param * (1.0 - my_sgrdel), y0, y0p, 1.0 - 0.5 * my_sgrdel, y1, y1p)
        else: # standard case
            sl_bar_del = (my_sldel - slr) / (1.0 - slr)
            s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
            a = (1 - s_gt_bar / (1.0 - sl_bar_del)) * math.pow(1.0 - math.pow(sl_bar + s_gt_bar, 1.0 / m), m)
            b = s_gt_bar / (1.0 - sl_bar_del) * math.pow(1.0 - math.pow(sl_bar_del, 1.0 / m), m)
    return math.sqrt(sl_bar) * math.pow(1.0 - a - b, 2)

def krel_liquid_prime(sl, slr, sgrdel, sgrmax, sldel, m, upper_liquid_param):
    if (sl < slr):
        return 0.0
    sl_bar = (sl - slr) / (1.0 - slr)
    sl_bar_prime = 1.0 / (1.0 - slr)
    if sgrdel == 0.0: # along the drying curve
        s_gt_bar = 0.0
        c = math.pow(sl_bar, 1.0 / m)
        dc_dsbar = c / m / sl_bar
        a = math.pow(1.0 - c, m)
        da_dsbar = - m * a / (1.0 - c) * dc_dsbar
        aprime = da_dsbar * sl_bar_prime
        b = 0.0
        bprime = 0.0
    else:
        if (sldel < slr): # turning point occured below slr, but "there is no hysteresis along the extension" according to p6 of Doughty2008.  The above checked that sl >= slr.  I assume the wetting relperm curve is the same as if the turning point = slr:
            my_sldel = slr
            my_sgrdel = sgrmax
        else:
            my_sldel = sldel
            my_sgrdel = sgrdel
        # this is Doughty2008: if sl >= (2.0 - upper_liquid_param) * (1.0 - my_sgrdel): # imporant for initialisation of cubic that this is >= and not >
        if sl >= 1.0 - 0.5 * my_sgrdel: # imporant for initialisation of cubic that this is >= and not >
            # follow the drying curve
            s_gt_bar = 0.0
            c = math.pow(sl_bar, 1.0 / m)
            dc_dsbar = c / m / sl_bar
            a = math.pow(1.0 - c, m)
            da_dsbar = -m * a / (1.0 - c) * dc_dsbar
            aprime = da_dsbar * sl_bar_prime
            b = 0.0
            bprime = 0.0
        elif sl > upper_liquid_param * (1.0 - my_sgrdel): # important that this is > and not >=
            return 100.0 # NOTE: this is incorrect, but it is never used
        else: # standard case
            sl_bar_del = (my_sldel - slr) / (1.0 - slr)
            s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
            s_gt_bar_prime = my_sgrdel / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
            c = math.pow(sl_bar + s_gt_bar, 1.0 / m)
            cprime = c / m / (sl_bar + s_gt_bar) * (sl_bar_prime + s_gt_bar_prime)
            a = (1 - s_gt_bar / (1.0 - sl_bar_del)) * math.pow(1.0 - c, m)
            aprime = -s_gt_bar_prime / (1.0 - sl_bar_del) * math.pow(1.0 - c, m) - m * a / (1.0 - c) * cprime
            b = s_gt_bar / (1.0 - sl_bar_del) * math.pow(1.0 - math.pow(sl_bar_del, 1.0 / m), m)
            bprime = s_gt_bar_prime * b / s_gt_bar
    kr = math.sqrt(sl_bar) * math.pow(1.0 - a - b, 2)
    return 0.5 * kr / sl_bar * sl_bar_prime - math.sqrt(sl_bar) * 2.0 * (1.0 - a - b) * (aprime + bprime)

def krel_gas(sl, slr, sgrdel, sgrmax, sldel, m, gamma, k_rg_max, y0p):
    if (sl < slr):
        if (k_rg_max == 1.0):
            return 1.0
        return cubic(sl, 0.0, 1.0, 0.0, slr, k_rg_max, y0p)
    sl_bar = (sl - slr) / (1.0 - slr)
    if (sgrdel == 0.0 or sl < sldel): # on the drying curve
        s_gt_bar = 0.0
    else:
        if (sldel < slr): # turning point occured below slr, but "there is no hysteresis along the extension" according to p6 of Doughty2008.  The above checked that sl >= slr.  I assume the wetting relperm curve is the same as if the turning point = slr:
            my_sldel = slr
            my_sgrdel = sgrmax
        else:
            my_sldel = sldel
            my_sgrdel = sgrdel
        s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
    if (sl_bar + s_gt_bar > 1.0):
        return 0.0 # always zero, irrespective of any hysteresis
    a = math.pow(1.0 - (sl_bar + s_gt_bar), gamma)
    c = math.pow(sl_bar + s_gt_bar, 1.0 / m)
    b = math.pow(1.0 - c, 2.0 * m)
    return k_rg_max * a * b

def krel_gas_prime(sl, slr, sgrdel, sgrmax, sldel, m, gamma, k_rg_max):
    if (sl < slr):
        if (k_rg_max == 1.0):
            return 0.0
        return 0.0 # NOTE: this is incorrect but it is never used
    if (sl > 1.0 - sgrdel):
        return 0.0 # always zero, irrespective of any hysteresis
    sl_bar = (sl - slr) / (1.0 - slr)
    sl_bar_prime = 1.0 / (1.0 - slr)
    if sgrdel == 0.0: # on the drying curve
        s_gt_bar = 0.0
        s_gt_bar_prime = 0.0
    else:
        if (sldel < slr): # turning point occured below slr, but "there is no hysteresis along the extension" according to p6 of Doughty2008.  The above checked that sl >= slr.  I assume the wetting relperm curve is the same as if the turning point = slr:
            my_sldel = slr
            my_sgrdel = sgrmax
        else:
            my_sldel = sldel
            my_sgrdel = sgrdel
        s_gt_bar = my_sgrdel * (sl - my_sldel) / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
        s_gt_bar_prime = my_sgrdel / (1.0 - slr) / (1.0 - my_sldel - my_sgrdel)
    a = math.pow(1.0 - (sl_bar + s_gt_bar), gamma)
    aprime = -gamma * a / (1.0 - (sl_bar + s_gt_bar)) * (sl_bar_prime + s_gt_bar_prime)
    c = math.pow(sl_bar + s_gt_bar, 1.0 / m)
    cprime = 1.0 / m * math.pow(sl_bar + s_gt_bar, 1.0 / m - 1.0) * (sl_bar_prime + s_gt_bar_prime)
    b = math.pow(1.0 - c, 2.0 * m)
    bprime = -2.0 * m * b / (1.0 - c) * cprime
    return k_rg_max * (a * bprime + aprime * b)


class Hys:
    def __init__(self, S_lr, m, gamma, k_rg_max, upper_liquid_param, S_lmin, S_grmax, alpha_d, alpha_w, n_d, n_w, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension):
        self.S_lr = S_lr # liquid residual saturation, where liquid relperm -> 0 and gas relperm = k_rg_max
        self.m = m # exponent in liquid relperm
        self.gamma = gamma # exponent in gas relperm
        self.k_rg_max = k_rg_max # maximum value of gas relperm (value attained S_liquid = S_lr)
        self.upper_liquid_param = upper_liquid_param # cubic spline extension for liquid relperm.  must be positive and <= 1. should be close to 1
        self.S_lmin = S_lmin # liquid saturation where van-Genuchten -> infinity
        self.S_grmax = S_grmax # gas saturation where wetting cap -> 0
        self.alpha_d = alpha_d # drying alpha
        self.alpha_w = alpha_w # wetting alpha
        self.n_d = n_d # drying n (= 1/(1 - m) for m being MOOSE param)
        self.n_w = n_w # wetting n (= 1/(1 - m) for m being MOOSE param)
        self.reinit_turning_points()
        self.absPcmax = absPcmax # defines point extension of capillary pressure at low saturation
        self.lower_extension_type = lower_extension_type # quadratic or exponential type of extension
        self.S_m_drying = sat_fcn(-self.absPcmax, self.S_lmin, 0.0, self.alpha_d, self.n_d, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # saturation at point of lower extension on primary drying curve
        self.dPc_dSm_drying = pc_fcn_prime(self.S_m_drying, self.S_lmin, 0.0, self.alpha_d, self.n_d, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # deriviative at point of lower extension on primary drying curve
        self.S_m_wetting = sat_fcn(-self.absPcmax, self.S_lmin, self.S_grmax, self.alpha_w, self.n_w, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # saturation at point of lower extension on primary wetting curve, with no upper extension at this point
        self.dPc_dSm_wetting = pc_fcn_prime(self.S_m_wetting, self.S_lmin, self.S_grmax, self.alpha_w, self.n_w, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # deriviative at point of lower extension on primary wetting curve, with no upper extension at this point
        self.upper_ratio = upper_ratio # must be positive and < 1.0
        self.upper_extension_type = upper_extension_type # if "power" then power-law extension of wetting curve at upper end, otherwise no extension
        self.S_m_upper = self.upper_ratio * (1.0 - self.S_grmax) # saturation at which upper extension starts on the primary wetting curve
        self.Pcm_upper = pc_fcn(self.S_m_upper, self.S_lmin, self.S_grmax, self.alpha_w, self.n_w, self.S_m_wetting, -self.absPcmax, self.dPc_dSm_wetting, self.lower_extension_type, self.S_m_upper, 0.0, 0.0, self.upper_extension_type) # pc on primary wetting curve at point of upper extension
        self.Pcmprime_upper = pc_fcn_prime(self.S_m_upper, self.S_lmin, self.S_grmax, self.alpha_w, self.n_w, self.S_m_wetting, -self.absPcmax, self.dPc_dSm_wetting, self.lower_extension_type, self.S_m_upper, self.Pcm_upper, 0.0, self.upper_extension_type) # dpc/ds on primary wetting curve at point of upper extension
        if (relperm_gas_extension == "linear_like"):
            self.krel_gas_prime = (self.k_rg_max - 1.0) / self.S_lr # this means the slope for the gas relperm at S_lr equals the slope of a straightline between (0, 1) and (S_lr, k_rg_max)
        else:
            self.krel_gas_prime = krel_gas_prime(self.S_lr, self.S_lr, 0.0, self.S_grmax, 1.0, self.m, self.gamma, self.k_rg_max) # This ensures the drying gas relperm is C1, but the wetting as relperm may have discontinuous deriv at self.S_lr if the turning point was less than self.S_lr

    def primary_drying(self, saturations):
        ''' Returns the absolute value of the primary drying capillary pressure values for the saturations.  There is no upper extension on this curve '''
        return [abs(pc_fcn(s, self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none")) for s in saturations]
    def primary_drying_sats(self, pcs):
        ''' Returns the absolute value of the saturations resulting from the pc values given, using the primary drying curve'''
        return [sat_fcn(pc, self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none") for pc in pcs]
    def primary_wetting(self, saturations):
        ''' Returns the absolute value of the primary wetting capillary pressure values for the saturations '''
        return [abs(pc_fcn(s, self.S_lmin, self.S_grmax, self.alpha_w, self.n_w, self.S_m_wetting, -self.absPcmax, self.dPc_dSm_wetting, self.lower_extension_type, self.S_m_upper, self.Pcm_upper, self.Pcmprime_upper, self.upper_extension_type)) for s in saturations]
    def relperm_liquid_drying(self, saturations):
        ''' Returns the liquid relperm during primary drying for the saturations. '''
        return [krel_liquid(s, self.S_lr, 0.0, self.S_grmax, 1.0, self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0) for s in saturations]
    def relperm_gas_drying(self, saturations):
        ''' Returns the gas relperm during primary drying for the saturations. '''
        return [krel_gas(s, self.S_lr, 0.0, self.S_grmax, 1.0, self.m, self.gamma, self.k_rg_max, self.krel_gas_prime) for s in saturations]

    def s_grdel_land(self, slDel):
        ''' Computes S_gr^Delta as per Eqn(2) of Doughty2007.  I *think* this is only for wetting (imbibition) curves, because it is zero for drying (drainage) curves '''
        a = 1.0 / self.S_grmax - 1.0 / (1.0 - self.S_lr)
        return (1.0 - slDel) / (1.0 + a * (1.0 - slDel))

    def reinit_turning_points(self):
        ''' Reinitialises the history of turning points '''
        self.S_lDel = [] # list of turning points
        self.Pcd_Del = [] # Pc(self.S_lDel) calculated using the primary drying curve
        self.gr_Del = [] # S_gr^Del(self.S_lDel) = Land S_gr^Del calculated at each turning point
        self.lc_Del = [] # 1 - self.gr_Del
        self.S_m_wetting_Del = [] # saturation at point of extension on the wetting curve defined by self.gr_Del
        self.dPc_dS_m_wetting_Del = [] # derivative at point of extension on the wetting curve defined by self.gr_Del
        self.sl_wetting_Del = [] # saturation on the wetting curve defined by self.gr_Del, at pc = self.Pcd_Del
        self.Pc_Del = [] # Pc calulcated by the scanning-curve procedure at self.S_lDel
        self.Sd_Del = [] # saturation on the drying curve using Pc_Del
        self.S_m_upper_Del = [] # saturation on the wetting curve defined by self.gr_Del at which upper extension starts
        self.Pcm_upper_Del = [] # Pc on the wetting curve defined by self.gr_Del, at saturation = self.S_m_upper_Del
        self.Pcmprime_upper_Del = [] # dPc/dS on the wetting curve defined by self.gr_Del, at saturation = self.S_m_upper_Del
        self.kl_begin_Del = [] # liquid relperm value at self.upper_liquid_param * self.lc_Del, used in cubic-spline transition from wetting to drying curve
        self.klp_begin_Del = [] # derivative of liquid relperm value at self.upper_liquid_param * self.lc_Del, used in cubic-spline transition from wetting to drying curve
        self.kl_end_Del = [] # liquid relperm value at (2 - self.upper_liquid_param) * self.lc_Del, used in cubic-spline transition from wetting to drying curve
        self.klp_end_Del = [] # derivative of liquid relperm value at (2 - self.upper_liquid_param) * self.lc_Del, used in cubic-spline transition from wetting to drying curve
        self.order = 0 # order of curve
        self.prev_saturation = 1.0 # previous saturation encountered
        self.latest_pc = 0.0 # latest value of Pc calculated
        self.latest_kl = 1.0 # latest value of liquid relperm calculated
        self.latest_kg = 0.0 # latest value of gas relperm calculated

    def pc(self, saturations, reinit_tp):
        ''' Computes pc given the saturation history contained in the "saturations" list.  if reinit_tp == true then the turning points and order are reinitialised to their 'primary drying' values '''
        if (reinit_tp):
            self.reinit_turning_points()
        if (len(saturations) == 0):
            return []

        self.set_pc_val(saturations[0])
        pc_vals = [self.latest_pc]
        self.prev_saturation = saturations[0]
        self.define_order(saturations[0])
        for s in saturations[1:]:
            self.define_order(s)
            self.set_pc_val(s)
            pc_vals.append(self.latest_pc)
        return pc_vals

    def set_pc_val(self, s):
        '''
        Sets self.latest_pc, which is the capillary pressure for the saturation, based on self.order
        '''
        if self.order == 0:
            self.latest_pc = pc_fcn(s, self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # no upper extension on primary drying curve
        elif self.order == 1:
            # define the interpolation: s_to_use smoothly transitions from Sl_wetting_Del[0] (the value of Sl on the wetting curve defined by self.gr_Del[0]) when s = S_lDel[0] (the value of Sl on the drying curve when we transitioned to wetting), to lc_Del[0] (the value of Sl at Pc=0 on the wetting curve defined by self.gr_Del[0]) when s = lc_Del[0]
            if self.upper_extension_type == "power":
                s_to_use = self.sl_wetting_Del[0] + (1.0 - self.sl_wetting_Del[0]) * (s - self.S_lDel[0]) / (1.0 - self.S_lDel[0])  # in this case lc_Del[0] is effectively 1.0 since the wetting capillary curve is extended to Sl=1
            else:
                s_to_use = self.sl_wetting_Del[0] + (self.lc_Del[0] - self.sl_wetting_Del[0]) * (s - self.S_lDel[0]) / (self.lc_Del[0] - self.S_lDel[0])
            self.latest_pc = pc_fcn(s_to_use, self.S_lmin, self.gr_Del[0], self.alpha_w, self.n_w, self.S_m_wetting_Del[0], -self.absPcmax, self.dPc_dS_m_wetting_Del[0], self.lower_extension_type, self.S_m_upper_Del[0], self.Pcm_upper_Del[0], self.Pcmprime_upper_Del[0], self.upper_extension_type)
        elif self.order == 2:
            # NOTE: here i think i don't use Eqn(3) from Niemi and Bodvarsson (which is Eqn(5) from Finsterle, Sonnenborg and Faybishenko).  I couldn't get their idea to work, so i made up something i think is sensible
            # The idea is to define capillary pressure using the primary drying curve, but use a different saturation than just "s".
            # This is similar to the order=1 case, where the capillary pressure is defined using the wetting curve (with sgrdel) but the saturation is not just "s", but s_to_use.

            # At the latest turning point:
            sdprime = self.Sd_Del[1] # saturation on the primary drying curve corresponding to Pc calculated at the latest turning point

            # At the previous turning point:
            sd = self.S_lDel[0] # saturation at the previous turning point

            # The idea is:
            # if s = self.S_lDel[1] then s_to_use = sdprime
            # if s = self.S_lDel[0] then s_to_use = sd
            # An alternative idea would be to use a quadratic that obeys the above constraints and has slope = 1 at s = self.S_lDel[0] because that would give a continuous slope of the curve at that point.  I don't know if that is advantagious or not
            s_to_use = sd + (s - self.S_lDel[0]) * (sdprime - sd) / (self.S_lDel[1] - self.S_lDel[0])
            self.latest_pc = pc_fcn(s_to_use, self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # no upper extension on drying curve

            # The following commented-out stuff is Niemi et al, i think
            # slc = 1.0 - sgrdel
            # pcplus = pc_fcn(s, self.S_lmin, 0.0, self.alpha_d, self.n_d) # on the original drying curve
            # slwplus = sat_fcn(pcplus, self.S_lmin, sgrdel, self.alpha_w, self.n_w) # on the wetting curve with S_gr = S_grdel but not the scanning version
            # lhs = (s - self.S_lDel[-1]) * pow(slc - slwplus, 2.0) / (slc - s) # MOOSE: be careful with s=slc
            # s_to_use = 0.5 * (slc + self.S_lDel[-1]) - 0.5 * math.sqrt(pow(slc - self.S_lDel[-1], 2.0) - 4 * lhs)
            # return pc_fcn(s_to_use, self.S_lmin, sgrdel, self.alpha_w, self.n_w)
        else:
            # Niemi and Bodvarsson aruge that a straight-line fit between the last two reversal points is appropriate, which is the approach taken by Finsterle, Sonnenborg and Faybishenko too, as well as Doughty2007 (except that in Dought2007 Fig5 doesn't look log-linear to me)
            # The following is the straight-line (in semi-log coords) but the results aren't that great
            # self.latest_pc = -math.exp(math.log(-self.Pc_Del[-1]) + (s - self.S_lDel[-1]) * (math.log(-self.Pc_Del[-2]) - math.log(-self.Pc_Del[-1])) / (self.S_lDel[-2] - self.S_lDel[-1]))
            # Instead, I interpolate between the previous first-order wetting and second-order trying curves
            if self.upper_extension_type == "power":
                s_to_use = self.sl_wetting_Del[0] + (1.0 - self.sl_wetting_Del[0]) * (s - self.S_lDel[0]) / (1.0 - self.S_lDel[0])
            else:
                s_to_use = self.sl_wetting_Del[0] + (self.lc_Del[0] - self.sl_wetting_Del[0]) * (s - self.S_lDel[0]) / (self.lc_Del[0] - self.S_lDel[0])
            first_order_wetting = pc_fcn(s_to_use, self.S_lmin, self.gr_Del[0], self.alpha_w, self.n_w, self.S_m_wetting_Del[0], -self.absPcmax, self.dPc_dS_m_wetting_Del[0], self.lower_extension_type, self.S_m_upper_Del[0], self.Pcm_upper_Del[0], self.Pcmprime_upper_Del[0], self.upper_extension_type)
            s_to_use = self.S_lDel[0] + (s - self.S_lDel[0]) * (self.Sd_Del[1] - self.S_lDel[0]) / (self.S_lDel[1] - self.S_lDel[0])
            second_order_drying = pc_fcn(s_to_use, self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none") # no upper extension on drying curve
            if (-first_order_wetting <= 0.0 or -second_order_drying <= 0.0):
                self.latest_pc = 0.0;
            else:
                self.latest_pc = -math.exp(math.log(-first_order_wetting) + (s - self.S_lDel[1]) * (math.log(-second_order_drying) - math.log(-first_order_wetting)) / (self.S_lDel[2] - self.S_lDel[1]))

    def relperms(self, saturations, reinit_tp):
        ''' Computes liquid and gas relperms given the saturation history contained in the "saturations" list.  if reinit_tp == true then the turning points and order are reinitialised to their 'primary drying' values '''
        if (reinit_tp):
            self.reinit_turning_points()
        if (len(saturations) == 0):
            return [[], []]

        self.set_relperm_vals(saturations[0])
        relperm_vals = [[self.latest_kl, self.latest_kg]]
        self.prev_saturation = saturations[0]
        self.define_order(saturations[0])
        for s in saturations[1:]:
            self.define_order(s)
            self.set_relperm_vals(s)
            relperm_vals.append([self.latest_kl, self.latest_kg])
        return relperm_vals

    def set_relperm_vals(self, s):
        '''
        Sets self.latest_kl and self.latest_kg, which are the liquid and gas relative permeabilities for the saturation, based on self.order
        '''
        if self.order == 0:
            self.latest_kl = krel_liquid(s, self.S_lr, 0.0, self.S_grmax, 1.0, self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0)
            self.latest_kg = krel_gas(s, self.S_lr, 0.0, self.S_grmax, 1.0, self.m, self.gamma, self.k_rg_max, self.krel_gas_prime)
        else:
            # min(self.gr_Del[0], self.S_grmax) is used to ensure the gr_Del used is never greater than grmax, which would occur if the turning point were less than S_lr
            self.latest_kl = krel_liquid(s, self.S_lr, min(self.gr_Del[0], self.S_grmax), self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param, self.kl_begin_Del[0], self.klp_begin_Del[0], self.kl_end_Del[0], self.klp_end_Del[0])
            self.latest_kg = krel_gas(s, self.S_lr, min(self.gr_Del[0], self.S_grmax), self.S_grmax, self.S_lDel[0], self.m, self.gamma, self.k_rg_max, self.krel_gas_prime)

    def order_and_turning_points(self, saturations):
        '''
        Returns: (order, list of turning points) for the list of liquid saturations in "saturations".  This is used for testing, not for generating capillary pressures.  Note: self.order and self.S_lDel are not reinitialised by this method, which allows successive calls to this method to be tested
        '''
        for s in saturations:
            self.define_order(s)
        return (self.order, self.S_lDel)

    def define_order(self, new_saturation):
        '''
        Based on new_saturation and self.prev_saturation, defines self.order.
        Also defines self.prev_saturation = new_saturation
        Also, if the order has changed, adds to self.S_lDel and other things
        Also, checks to see if the order can be reduced and self.S_lDel can be reduced
        '''
        dry2wet = (self.order % 2 == 0) and (new_saturation > self.prev_saturation) # have been reducing saturation ("drying" or "draining") but now are increasing saturation ("wetting" or "imbibing")
        wet2dry = (self.order % 2 == 1) and (new_saturation < self.prev_saturation) # have been increasing saturation ("wetting" or "imbibing") but now are decreasing saturation ("drying" or "draining")
        if dry2wet or wet2dry:
            self.order += 1
            self.S_lDel.append(self.prev_saturation)
            self.Pcd_Del.append(pc_fcn(self.S_lDel[-1], self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none")) # no upper extension on drying curve
            self.gr_Del.append(self.s_grdel_land(self.S_lDel[-1]))
            self.lc_Del.append(1.0 - self.gr_Del[-1])
            self.S_m_wetting_Del.append(sat_fcn(-self.absPcmax, self.S_lmin, self.gr_Del[-1], self.alpha_w, self.n_w, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none")) # no upper extension at this point of computation
            self.dPc_dS_m_wetting_Del.append(pc_fcn_prime(self.S_m_wetting_Del[0], self.S_lmin, self.gr_Del[-1], self.alpha_w, self.n_w, 0.0, -self.absPcmax, 0.0, self.lower_extension_type, 1.0, 0.0, 0.0, "none")) # no upper extension at this point of computation
            self.sl_wetting_Del.append(sat_fcn(self.Pcd_Del[-1], self.S_lmin, self.gr_Del[-1], self.alpha_w, self.n_w, self.S_m_wetting_Del[-1], -self.absPcmax, self.dPc_dS_m_wetting_Del[-1], self.lower_extension_type, 1.0, 0.0, 0.0, "none")) # no upper extension at this point of computation
            self.Pc_Del.append(self.latest_pc)
            self.Sd_Del.append(sat_fcn(self.Pc_Del[-1], self.S_lmin, 0.0, self.alpha_d, self.n_d, self.S_m_drying, -self.absPcmax, self.dPc_dSm_drying, self.lower_extension_type, 1.0, 0.0, 0.0, "none")) # no upper extension on drying curve
            self.S_m_upper_Del.append(self.upper_ratio * (1.0 - self.gr_Del[-1]))
            self.Pcm_upper_Del.append(pc_fcn(self.S_m_upper_Del[-1], self.S_lmin, self.gr_Del[-1], self.alpha_w, self.n_w, self.S_m_wetting_Del[-1], -self.absPcmax, self.dPc_dS_m_wetting_Del[-1], self.lower_extension_type, self.S_m_upper_Del[-1], 0.0, 0.0, self.upper_extension_type))
            self.Pcmprime_upper_Del.append(pc_fcn_prime(self.S_m_upper_Del[-1], self.S_lmin, self.gr_Del[-1], self.alpha_w, self.n_w, self.S_m_wetting_Del[-1], -self.absPcmax, self.dPc_dS_m_wetting_Del[-1], self.lower_extension_type, self.S_m_upper_Del[-1], self.Pcm_upper_Del[-1], 0.0, self.upper_extension_type))
            # Relperm stuff
            # only use [0] because there is only one wetting curve
            if (self.S_lDel[0] < self.S_lr): # deal with the case that "there is no hysteresis along the extension"
                self.kl_begin_Del.append(krel_liquid(self.upper_liquid_param * (1.0 - self.S_grmax), self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                self.klp_begin_Del.append(krel_liquid_prime(self.upper_liquid_param * (1.0 - self.S_grmax), self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param))
                # Doughty2008: self.kl_end_Del.append(krel_liquid((2.0 - self.upper_liquid_param) * (1.0 - self.S_grmax), self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                # Doughty2008: self.klp_end_Del.append(krel_liquid_prime((2.0 - self.upper_liquid_param) * (1.0 - self.S_grmax), self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param))
                self.kl_end_Del.append(krel_liquid(1.0 - 0.5 * self.S_grmax, self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                self.klp_end_Del.append(krel_liquid_prime(1.0 - 0.5 * self.S_grmax, self.S_lr, self.S_grmax, self.S_grmax, self.S_lr, self.m, self.upper_liquid_param))
            else:
                self.kl_begin_Del.append(krel_liquid(self.upper_liquid_param * self.lc_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                self.klp_begin_Del.append(krel_liquid_prime(self.upper_liquid_param * self.lc_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param))
                # Doughty2008: self.kl_end_Del.append(krel_liquid((2.0 - self.upper_liquid_param) * self.lc_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                # Doughty2008: self.klp_end_Del.append(krel_liquid_prime((2.0 - self.upper_liquid_param) * self.lc_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param))
                self.kl_end_Del.append(krel_liquid(1.0 - 0.5 * self.gr_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param, 0.0, 0.0, 0.0, 0.0))
                self.klp_end_Del.append(krel_liquid_prime(1.0 - 0.5 * self.gr_Del[0], self.S_lr, self.gr_Del[0], self.S_grmax, self.S_lDel[0], self.m, self.upper_liquid_param))
        can_reduce = self.order > 1
        while (can_reduce):
            can_reduce = False
            reducing_and_low_sat = (self.order % 2 == 0) and (new_saturation <= self.S_lDel[-2]) # are reducing saturation and new_saturation is <= a previously-encountered turning point
            increasing_and_high_sat = (self.order % 2 == 1) and (new_saturation >= self.S_lDel[-2]) # are increasing saturation and new_saturation is >= a previously-encountered turning point
            if reducing_and_low_sat or increasing_and_high_sat:
                self.order -= 2
                for i in range(2):
                    self.S_lDel.pop()
                    self.Pcd_Del.pop()
                    self.gr_Del.pop()
                    self.lc_Del.pop()
                    self.sl_wetting_Del.pop()
                    self.Pc_Del.pop()
                    self.Sd_Del.pop()
                    self.S_m_wetting_Del.pop()
                    self.dPc_dS_m_wetting_Del.pop()
                    self.S_m_upper_Del.pop()
                    self.Pcm_upper_Del.pop()
                    self.Pcmprime_upper_Del.pop()
                    self.kl_begin_Del.pop()
                    self.klp_begin_Del.pop()
                    self.kl_end_Del.pop()
                    self.klp_end_Del.pop()
                can_reduce = self.order > 1
        self.prev_saturation = new_saturation

Slmin = 0.1
Slr = 0.2 # krel_liquid(Slr) = 0, krel_gas(Slr) = krgmax.  Slr >= Slmin
m = 0.9 # exponent in liquid relperm < 1
gamma = 0.3 # exponent in gas relperm (0.3 -> 0.5 is reasonable)
krgmax = 0.8 # value of gas relperm when S_liquid = S_lr.  <=1
relperm_gas_extension = "linear_like" # the cubic is more like a line between (0, 1) and (Slr, krgmax)
Sgrmax = 0.3 # must be > 0 to ensure cubic spline works OK on liquid relperm curve
alphad = 5.0
alphaw = 10.0
nd = 1.7
nw = 1.9 # 1.1 is good for testing
absPcmax = 1.5 # value at which extension starts.  Because alphad!=alphaw and nd!=nw this causes the drying curve to exceed the wetting curve!  MOOSE note: for the quadratic extension, TOUGH demands that users specify |Pc(S=0)| which might be more user-friendly
lower_extension_type = "exponential"
upper_ratio = 0.9 # upper extension will start at S_l = 0.9 * (1 - Sgrmax) for the primary wetting curve
upper_extension_type = "power"
upper_liquid_param = 0.9 # cubic spline is between upper_liquid_param * (1 - Sgr_del) and 1 - 0.5 * Sgr_del.  NOTE: Doughty2008 sets this upper number to (2.0 - upper_liquid_param) * (1 - Sgr_del), but that can be >1 and i don't think it gives such a good result
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)

#### Testing of calculating order and the turning points
(order, S_ldel) = hys.order_and_turning_points([1.0])
assert order == 0 and len(S_ldel) == 0
(order, S_ldel) = hys.order_and_turning_points([0.9, 0.8, 0.7, 0.6, 0.5])
assert order == 0 and len(S_ldel) == 0
(order, S_ldel) = hys.order_and_turning_points([0.6, 0.7])
assert order == 1 and len(S_ldel) == 1 and S_ldel[0] == 0.5
(order, S_ldel) = hys.order_and_turning_points([0.7, 0.8, 0.9])
assert order == 1 and len(S_ldel) == 1 and S_ldel[0] == 0.5
(order, S_ldel) = hys.order_and_turning_points([0.8, 0.7, 0.6])
assert order == 2 and len(S_ldel) == 2 and S_ldel[0] == 0.5 and S_ldel[1] == 0.9
(order, S_ldel) = hys.order_and_turning_points([0.5])
assert order == 0 and len(S_ldel) == 0
(order, S_ldel) = hys.order_and_turning_points([0.4])
assert order == 0 and len(S_ldel) == 0
(order, S_ldel) = hys.order_and_turning_points([0.9, 0.6, 0.8, 0.7])
assert order == 4 and len(S_ldel) == 4 and S_ldel[0] == 0.4 and S_ldel[1] == 0.9 and S_ldel[2] == 0.6 and S_ldel[3] == 0.8
(order, S_ldel) = hys.order_and_turning_points([0.85])
assert order == 3 and len(S_ldel) == 3 and S_ldel[0] == 0.4 and S_ldel[1] == 0.9 and S_ldel[2] == 0.6
(order, S_ldel) = hys.order_and_turning_points([0.2])
assert order == 0 and len(S_ldel) == 0

eps = 1E-8
assert cubic(1, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0) == 2.0
deriv = (cubic(1.0 + eps, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0) - cubic(1.0 - eps, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0)) / 2.0 / eps
assert deriv < 3 + 10 * eps and deriv > 3 - 10 * eps
assert cubic(4, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0) == 5.0
deriv = (cubic(4.0 + eps, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0) - cubic(4.0 - eps, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0)) / 2.0 / eps
assert deriv < 6 + 10 * eps and deriv > 6 - 10 * eps


#### Plot the primary drying and wetting curves
plt.figure(0)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Wetting")
plt.semilogy([Slmin, Slmin], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(Slmin, 0.01, "$S_{lmin}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - Sgrmax, 1 - Sgrmax], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(1 - Sgrmax, 0.01, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Primary drying and wetting curves")

#### Plot hysteretic curves
plt.figure(1)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
plt.semilogy([Slmin, Slmin], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(Slmin, 0.01, "$S_{lmin}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - Sgrmax, 1 - Sgrmax], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(1 - Sgrmax, 0.01, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.5 is the turning point in the following
svals = [1 - 0.001 * i for i in range(500)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "hysteretic 0")
plt.semilogy(svals[-1], pcvals[-1], 'rs', label = "turning point 0")
svals = [0.5 + 0.001 * i for i in range(500)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "hysteretic 1")
plt.semilogy([1.0 - hys.s_grdel_land(0.5)], [0.01], 'gs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure: first order")

#### Plot hysteretic curves
plt.figure(2)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
plt.semilogy([Slmin, Slmin], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(Slmin, 0.01, "$S_{lmin}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - Sgrmax, 1 - Sgrmax], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(1 - Sgrmax, 0.01, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.2 and 0.65 are the turning points in the following
svals = [1 - 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "hysteretic 0")
plt.semilogy(svals[-1], pcvals[-1], 'rs', label = "turning point 0")
svals = [0.2 + 0.001 * i for i in range(450)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "hysteretic 1")
plt.semilogy(svals[-1], pcvals[-1], 'r.', label = "turning point 1")
svals = [0.65 - 0.001 * i for i in range(550)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g:', label = "hysteretic 2")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure: second order")


#### Plot hysteretic curves
plt.figure(3)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
plt.semilogy([Slmin, Slmin], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(Slmin, 0.01, "$S_{lmin}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - Sgrmax, 1 - Sgrmax], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(1 - Sgrmax, 0.01, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.2 and 0.65 and 0.25 are the turning points in the following
svals = [1 - 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "hysteretic 0")
svals = [0.2 + 0.001 * i for i in range(450)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "hysteretic 1")
svals = [0.65 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g:', label = "hysteretic 2")
svals = [0.25 + 0.001 * i for i in range(500)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-.', label = "hysteretic 3")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure: third order")
#plt.savefig("cp_3.png")


#### Plot hysteretic curves
plt.figure(4)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
plt.semilogy([Slmin, Slmin], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(Slmin, 0.01, "$S_{lmin}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - Sgrmax, 1 - Sgrmax], [0, max(hys.primary_drying(svals))], 'k:')
plt.text(1 - Sgrmax, 0.01, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.2 and 0.65 and 0.25 and 0.55 are the turning points in the following
svals = [1 - 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "hysteretic 0")
svals = [0.2 + 0.001 * i for i in range(450)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "hysteretic 1")
svals = [0.65 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g:', label = "hysteretic 2")
svals = [0.25 + 0.001 * i for i in range(300)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-.', label = "hysteretic 3")
svals = [0.55 - 0.03 * i for i in range(15)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'ko', markerfacecolor = 'none', label = "hysteretic 4, etc")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure: fourth order")

#### Plot the drying relperm
plt.figure(5)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Drying relative permeability functions")

#### Plot hysteretic curves
plt.figure(6)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.5 is the turning point in the following
svals = [1 - 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "liquid 0")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "gas 0")
plt.plot(svals[-1], kvals[-1][0], 'rs')
plt.plot(svals[-1], kvals[-1][1], 'rs')
svals = [0.5 + 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'g-', label = "liquid 1")
plt.plot(svals, [k[1] for k in kvals], 'b-', label = "gas 1")
plt.plot([1.0 - hys.s_grdel_land(0.5)], [0.0], 'gs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.grid()
plt.xlim([0, 1])
##plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Relative permeability functions")

#### Plot hysteretic liquid relperm
plt.figure(7)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.35 and 0.9 and 0.4 and 0.55 are the turning points in the following
svals = [1 - 0.001 * i for i in range(650)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "hysteretic 0")
svals = [0.35 + 0.001 * i for i in range(550)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'y-', label = "hysteretic 1")
svals = [0.9 - 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'g:', label = "hysteretic 2")
svals = [0.4 + 0.001 * i for i in range(150)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'g-.', label = "hysteretic 3")
svals = [0.55 - 0.03 * i for i in range(15)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'ko', markerfacecolor = 'none', label = "hysteretic 4, etc")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Liquid relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic liquid relative permeability: fourth order")


#### Plot hysteretic gas relperm
plt.figure(8)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_gas_drying(svals), 'r-', label = "Drying")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.25 and 0.9 and 0.4 and 0.55 are the turning points in the following
svals = [1 - 0.001 * i for i in range(750)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[1] for k in kvals], 'g--', label = "hysteretic 0")
svals = [0.25 + 0.001 * i for i in range(650)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'y-', label = "hysteretic 1")
svals = [0.9 - 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'g:', label = "hysteretic 2")
svals = [0.4 + 0.001 * i for i in range(150)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'g-.', label = "hysteretic 3")
svals = [0.55 - 0.03 * i for i in range(15)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'ko', markerfacecolor = 'none', label = "hysteretic 4, etc")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Gas relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic gas relative permeability: fourth order")

#### Plot hysteretic gas relperm
plt.figure(9)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_gas_drying(svals), 'r-', label = "Drying")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.6 is the turning points in the following
svals = [1 - 0.001 * i for i in range(400)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[1] for k in kvals], 'g--', label = "hysteretic 0")
svals = [0.6 + 0.001 * i for i in range(400)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'y-', label = "hysteretic 1")
plt.plot([1.0 - hys.s_grdel_land(0.6)], [0.0], 'gs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Gas relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic gas relative permeability: first order")

#### Plot hysteretic gas relperm
plt.figure(10)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_gas_drying(svals), 'r-', label = "Drying")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.05 is the turning points in the following
svals = [1 - 0.001 * i for i in range(950)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[1] for k in kvals], 'g--', label = "hysteretic 0")
svals = [0.05 + 0.001 * i for i in range(950)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'y-', label = "hysteretic 1")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Gas relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic gas relative permeability: first order")


#### Plot hysteretic water relperm
plt.figure(11)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, 0.0, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, 0.0, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.05 is the turning points in the following
svals = [1 - 0.001 * i for i in range(950)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "hysteretic 0")
svals = [0.05 + 0.001 * i for i in range(950)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'y-', label = "hysteretic 1")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Liquid relative permeability")
plt.grid()
plt.xlim([0, 1])
#plt.ylim([0, 1])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic water relative permeability: first order")

#### Plot hysteretic curves to output nice figures for MOOSE documentation
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, 10.0, lower_extension_type, 0.999, upper_extension_type, relperm_gas_extension)
plt.figure(12)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
# 0.18 and 0.65 and 0.25 are the turning points in the following
svals = [1 - 0.001 * i for i in range(820)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "order 0")
plt.semilogy([0.18, 0.18], [0, pcvals[-1]], 'k:')
plt.text(0.18, 0.01, "TP$_{0}\,$", horizontalalignment='right', verticalalignment='bottom', fontsize = 16)
svals = [0.18 + 0.001 * i for i in range(470)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "order 1")
plt.semilogy([0.65, 0.65], [0, pcvals[-1]], 'k:')
plt.text(0.65, 0.01, "TP$_{1}$", horizontalalignment='right', verticalalignment='bottom', fontsize = 16)
svals = [0.65 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g:', label = "order 2")
plt.semilogy([0.25, 0.25], [0, pcvals[-1]], 'k:')
plt.text(0.25, 0.01, "TP$_{2}\,$", horizontalalignment='left', verticalalignment='bottom', fontsize = 16)
svals = [0.25 + 0.001 * i for i in range(300)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-.', label = "order 3")
plt.xlabel("Liquid saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure")
#plt.savefig("hysteretic_order.png")

hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, 10.0, lower_extension_type, 0.999, upper_extension_type, relperm_gas_extension)
plt.figure(13)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
svals = [1 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "(a) order 0")
svals = [0.6 + 0.001 * i for i in range(300)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "(b) order 1")
svals = [0.9 - 0.001 * i for i in range(300)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g:', label = "(c) order 2")
svals = [0.6 - 0.001 * i for i in range(300)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g--', label = "(c) order 0")
svals = [0.3 + 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'y-', label = "(d) order 1")
svals = [0.7 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'y:', label = "(e) order 2")
svals = [0.3 - 0.001 * i for i in range(100)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'y--', label = "(e) order 0")
svals = [0.2 + 0.001 * i for i in range(200)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'c-', label = "(f) order 1")
svals = [0.4 - 0.001 * i for i in range(200)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'c:', label = "(g) order 2")
svals = [0.2 - 0.001 * i for i in range(100)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'c--', label = "(g) order 0")
plt.xlabel("Liquid saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic capillary pressure")
#plt.savefig("hysteretic_order_012.png")


plt.figure(14)
svals = [0.001 * i for i in range(1000)]
this_Slmin = 0.1
this_Sgrmax = 0.15
this_n = 3.0
plt.semilogy([this_Slmin, this_Slmin], [0, 1E5], 'k:')
plt.text(this_Slmin, 0.01, "$S_{l,min}$", horizontalalignment='center', verticalalignment='top')
plt.semilogy([1 - this_Sgrmax, 1 - this_Sgrmax], [0, 1E5], 'k:')
plt.text(1 - this_Sgrmax, 0.01, "$1 - S_{gr}^{max}$", horizontalalignment='center', verticalalignment='top')
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, this_Slmin, this_Sgrmax, alphad, alphad, this_n, this_n, 1.1, "quadratic", 0.999999, "none", relperm_gas_extension)
plt.semilogy(svals, hys.primary_wetting(svals), 'r--', label = "Quadratic lower")
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, this_Slmin, this_Sgrmax, alphad, alphad, this_n, this_n, 1.1, "exponential", 0.9999999, "none", relperm_gas_extension)
plt.semilogy(svals, hys.primary_wetting(svals), 'g-.', label = "Exponential lower")
xval = hys.S_m_wetting
plt.semilogy([0, xval], [1.1, 1.1], 'k:')
plt.text(xval, 1.1, " " + "$P_{c}^{max}$", horizontalalignment='left', verticalalignment='center')
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, this_Slmin, this_Sgrmax, alphad, alphad, this_n, this_n, 1E4, "none", 0.9, "power", relperm_gas_extension)
plt.semilogy(svals, hys.primary_wetting(svals), 'c--', label = "Power upper")
plt.semilogy([0.9 * (1 - this_Sgrmax), 0.9 * (1 - this_Sgrmax)], [0, hys.primary_wetting([0.9 * (1 - this_Sgrmax)])[0]], 'k:')
plt.text(0.9 * (1 - this_Sgrmax), hys.primary_wetting([0.9 * (1 - this_Sgrmax)])[0], " " + "$0.9 (1 - S_{gr}^{max})$", horizontalalignment='center', verticalalignment='bottom', rotation=90)
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, this_Slmin, this_Sgrmax, alphad, alphad, this_n, this_n, 1000.0, "none", 0.9999999, "none", relperm_gas_extension)
plt.semilogy(svals, hys.primary_wetting(svals), 'k-', label = "No extension")
plt.xlabel("Liquid saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xticks([0, 0.3, 0.5, 0.7, 1], ["0", "0.3", "0.5", "0.7", "1"])
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Primary wetting capillary pressure with various extensions")
#plt.savefig("hysteretic_cap_extensions.png")


hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, 10.0, lower_extension_type, 0.999, "none", relperm_gas_extension)
plt.figure(15)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Primary wetting")
plt.text(1 - Sgrmax, 0.01, "$1 - S_{gr}^{max}$", horizontalalignment='center', verticalalignment='top')
plt.text(Slmin, 0.01, "$S_{l,min}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(100)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
svals = [0.9 + 0.001 * i for i in range(100)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "order 1")
lc = [hys.lc_Del[0]]
svals = [1 - 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
svals = [0.6 + 0.001 * i for i in range(400)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
lc.append(hys.lc_Del[0])
plt.semilogy(svals, pcvals, 'g-', label = "order 1")
svals = [1 - 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
svals = [0.2 + 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
lc.append(hys.lc_Del[0])
plt.semilogy(svals, pcvals, 'g-', label = "order 1")
plt.semilogy([lc[0]], [0.01], 'cs', label = "$1 - S_{gr}^{\Delta}$")
plt.semilogy([lc[1]], [0.01], 'cs')
plt.semilogy([lc[2]], [0.01], 'cs')
plt.xlabel("Liquid saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Various first-order wetting curves")
#plt.savefig("hysteretic_different_tps.png")


hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, 10.0, lower_extension_type, 0.999, "none", relperm_gas_extension)
plt.figure(16)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_drying(svals), 'r-', label = "Primary drying")
plt.semilogy([Slmin, Slmin], [0.01, 1E4], 'k:')
plt.text(Slmin, 0.01, "$S_{l,min}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(600)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
svals = [0.4 + 0.001 * i for i in range(600)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "First-order curve")
plt.semilogy([0.4, 0.4, 0], [0.01, -hys.Pcd_Del[0], -hys.Pcd_Del[0]], 'k:')
plt.semilogy([hys.sl_wetting_Del[0], hys.sl_wetting_Del[0], 0], [0.01, -hys.Pcd_Del[0], -hys.Pcd_Del[0]], 'k:')
plt.text(0.4, 0.01, "TP$_{0}$", horizontalalignment='center', verticalalignment='top')
plt.text(hys.sl_wetting_Del[0], 0.01, "$S_{l,wet}^{\Delta}$", horizontalalignment='center', verticalalignment='top')
plt.text(0, -hys.Pcd_Del[0], "$P_{c}^{\Delta}$", horizontalalignment='right', verticalalignment='center')
gr_del = hys.gr_Del[0]
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, gr_del, alphad, alphaw, nd, nw, 10.0, lower_extension_type, 0.999, "none", relperm_gas_extension)
svals = [0.001 * i for i in range(1000)]
plt.semilogy(svals, hys.primary_wetting(svals), 'b-', label = "Wetting curve using $S_{gr}^{\Delta}$")
plt.semilogy([1 - gr_del], [0.01], 'cs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Liquid saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xticks([0, 0.3, 0.5, 0.7, 1], ["0", "0.3", "0.5", "0.7", "1"])
plt.yticks([0.01, 0.1, 10], ["0.01", "0.1", "10"])
plt.xlim([0, 1])
plt.ylim([0.01, 10])
plt.legend()
plt.tight_layout()
plt.title("Construction of the first-order wetting curve")
#plt.savefig("hysteretic_order1.png")


#### Plot drying and wetting relperms with no extensions
upper_liquid_param = 1.0 # below ignores upper extension on liquid wetting curve
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, "no_relperm_gas_extension")
plt.figure(17)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
svals = [0.001 * i for i in range(200, 1001)]
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([0, 0.2], [krgmax, krgmax], 'r:')
plt.text(0, krgmax, "$k_{r,g}^{max}$", horizontalalignment='right', verticalalignment='center')
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{gr}^{max}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, True)
plt.plot(svals[-1], kvals[-1][0], 'rs', label = "turning point")
svals = [0.2 + 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, False)
svals += [0.2 + 0.001 * i for i in range(500, 800)]
rpvals = [k[0] for k in kvals] + [1.0 for s in range(500, 800)]
plt.plot(svals, rpvals, 'b-', label = "first-order, liquid")
svals = [1 - 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, True)
plt.plot(svals[-1], kvals[-1][1], 'rs')
svals = [0.2 + 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'b:', label = "first-order, gas")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.grid()
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "", "1"])
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-2])
plt.legend()
#plt.tight_layout()
plt.title("Unmodified relative permeability functions")
#plt.savefig("hysteretic_krel_unextended.png")


upper_liquid_param = 0.9
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)
plt.figure(18)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.text(0, krgmax, "$k_{r,g}^{max}$", horizontalalignment='right', verticalalignment='center')
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{gr}^{max}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, True)
plt.plot(svals[-1], kvals[-1][0], 'rs', label = "turning point")
svals = [0.2 + 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'b-', label = "first-order, liquid")
svals = [1 - 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, True)
plt.plot(svals[-1], kvals[-1][1], 'rs')
svals = [0.2 + 0.001 * i for i in range(800)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[1] for k in kvals], 'b:', label = "first-order, gas")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.grid()
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "", "1"])
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("Relative permeability functions with extensions")
#plt.savefig("hysteretic_krel_extended.png")

plt.figure(19)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.5 is the turning point in the following
svals = [1 - 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "liquid 0")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "gas 0")
plt.plot(svals[-1], kvals[-1][0], 'rs')
plt.plot(svals[-1], kvals[-1][1], 'rs')
svals = [0.5 + 0.001 * i for i in range(500)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'g-', label = "liquid 1")
plt.plot(svals, [k[1] for k in kvals], 'b-', label = "gas 1")
plt.plot([1.0 - hys.s_grdel_land(0.5)], [0.0], 'gs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "", "1"])
plt.grid()
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic relative permeability functions")
#plt.savefig("hysteretic_krel_example_1.png")


plt.figure(20)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
# 0.1 is the turning point in the following
svals = [1 - 0.001 * i for i in range(900)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "liquid 0")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "gas 0")
plt.plot(svals[-1], kvals[-1][0], 'rs')
plt.plot(svals[-1], kvals[-1][1], 'rs')
svals = [0.1 + 0.001 * i for i in range(900)]
kvals = hys.relperms(svals, False)
plt.plot(svals, [k[0] for k in kvals], 'g-', label = "liquid 1")
plt.plot(svals, [k[1] for k in kvals], 'b-', label = "gas 1")
plt.plot([1.0 - min(hys.gr_Del[0], hys.S_grmax)], [0.0], 'gs', label = "$1 - S_{gr}^{\Delta}$")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "", "1"])
plt.grid()
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("Hysteretic relative permeability functions")
#plt.savefig("hysteretic_krel_example_2.png")

# For comparison with the vary_sat series of tests
Slmin = 0.1
Slr = 0.2
Sgrmax = 0.3
alphad = 10.0
alphaw = 7.0
nd = 1.5
nw = 1.9
absPcmax = 12
lower_extension_type = "quadratic"
upper_ratio = 0.9
upper_extension_type = "power"
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)

plt.figure(21)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
f = open("gold/vary_sat_1_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
#plt.semilogy([x[3] for x in data], [x[2] for x in data], 'b.')

svals = [1 - 0.001 * i for i in range(1000)] + [0.0 + 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying then wetting")
f = open("gold/vary_sat_1b.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[3] for x in data], [x[2] for x in data], 'g+', label="MOOSE")

svals = [1 - 0.001 * i for i in range(800)] + [0.2 + 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'y-.', label = "drying then first-order wetting")
f = open("gold/vary_sat_1c.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[3] for x in data], [x[2] for x in data], 'y*', label="MOOSE")
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure examples")
#plt.savefig("../../../doc/content/media/porous_flow/hys_vary_sat_1.png")


plt.figure(22)
svals = [1 - 0.001 * i for i in range(800)] + [0.2 + 0.001 * i for i in range(600)] + [0.8 - 0.001 * i for i in range(800)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'k-', label = 'Expected')
f = open("gold/vary_sat_1d.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[3] for x in data], [x[2] for x in data], 'b.', label = 'MOOSE')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure 2nd-order example")
#plt.savefig("../../../doc/content/media/porous_flow/hys_vary_sat_1_2ndorder.png")

plt.figure(23)
svals = [1 - 0.001 * i for i in range(800)] + [0.2 + 0.001 * i for i in range(600)] + [0.8 - 0.001 * i for i in range(500)] + [0.3 + 0.001 * i for i in range(700)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'k-', label = 'Expected')
f = open("gold/vary_sat_1e.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[3] for x in data], [x[2] for x in data], 'b.', label = 'MOOSE')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure 3rd-order example")
#plt.savefig("../../../doc/content/media/porous_flow/hys_vary_sat_1_3rdorder.png")

plt.figure(24)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(905)] + [0.096 + 0.001 * i for i in range(905)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying then wetting")
f = open("gold/1phase_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[4] for x in data], [max(-x[3], 0) for x in data], 'g+', label = 'MOOSE')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure examples")
#plt.savefig("../../../doc/content/media/porous_flow/hys_1phase_1.png")

plt.figure(25)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(905)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
svals = [0.096 + 0.001 * i for i in range(705)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'g-', label = "1st-order wetting")
svals = [0.8 - 0.001 * i for i in range(605)]
pcvals = [abs(pc) for pc in hys.pc(svals, False)]
plt.semilogy(svals, pcvals, 'k-', label = "2nd-order drying")
# do not plot hys.pc since the third-order S(Pc) is not an inverse of Pc(S)
#svals = [0.196 + 0.001 * i for i in range(605)]
#pcvals = [abs(pc) for pc in hys.pc(svals, False)]
#plt.semilogy(svals, pcvals, 'y-', label = "3rd order")
#instead, read from a by-hand calculation:
f = open("3rd_order_eg.txt", "r")
data = [list(map(float, line.strip().split())) for line in f.readlines()]
plt.semilogy([x[0] for x in data], [x[1] for x in data], 'y-', label = '3rd order')
f.close()
f = open("gold/1phase_3rd_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[4] for x in data], [max(-x[3], 0) for x in data], 'r+', label = 'MOOSE')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.001, 40])
plt.legend()
plt.title("Hysteretic capillary pressure example: 3rd order")
#plt.savefig("../../../doc/content/media/porous_flow/hys_1phase_3.png")

plt.figure(26)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(465)] + [0.535 + 0.001 * i for i in range(465)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying then wetting")
f = open("gold/2phasePP_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[5] for x in data], [x[4] - x[3] for x in data], 'g+', label = 'MOOSE, 2 phase PP')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0.4, 1])
plt.ylim([0.01, 1])
plt.legend()
plt.title("Hysteretic capillary pressure examples (2 phase)")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePP_1.png")

plt.figure(27)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(512)] + [0.512 + 0.001 * i for i in range(440)] + [0.952 - 0.001 * i for i in range(500)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying, wetting, re-drying")
f = open("gold/2phasePP_2_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[5] for x in data], [x[4] - x[3] for x in data], 'g+', label = 'MOOSE, 2 phase PP')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure examples (2 phase)")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePP_2.png")

plt.figure(28)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(465)] + [0.535 + 0.001 * i for i in range(465)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying then wetting")
f = open("gold/2phasePS_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[5] for x in data], [x[4] - x[3] for x in data], 'g+', label = 'MOOSE, 2 phase PS')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0, 1])
plt.ylim([0.01, 40])
plt.legend()
plt.title("Hysteretic capillary pressure examples (2 phase)")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_1.png")

plt.figure(29)
svals = [1 - 0.001 * i for i in range(1000)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'b-', label = "primary drying")
svals = [1 - 0.001 * i for i in range(512)] + [0.512 + 0.001 * i for i in range(440)] + [0.952 - 0.001 * i for i in range(500)]
pcvals = [abs(pc) for pc in hys.pc(svals, True)]
plt.semilogy(svals, pcvals, 'g--', label = "drying, wetting, re-drying")
f = open("gold/2phasePS_2_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.semilogy([x[5] for x in data], [x[4] - x[3] for x in data], 'g+', label = 'MOOSE, 2 phase PS')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Capillary pressure ($|P_{c}|$)")
plt.grid("x")
plt.xlim([0.4, 1])
plt.ylim([0.001, 1])
plt.legend()
plt.title("Hysteretic capillary pressure examples (2 phase)")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_2.png")

# For comparison with the relperm series of tests
Slmin = 0.1
Sgrmax = 0.3
alphad = 10.0
alphaw = 10.0
nd = 1.5
nw = 1.5
absPcmax = 1E10
lower_extension_type = "quadratic"
upper_extension_type = "power"
Slr = 0.1
Sgrmax = 0.2
m = 0.9
upper_ratio = 0.9
gamma = 0.33
krgmax = 0.8
relperm_gas_extension = "linear_like"
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)

plt.figure(30)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(502)] + [0.498 + 0.001 * i for i in range(502)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "expected")
f = open("gold/1phase_relperm_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[4] for x in data], [x[3] for x in data], 'g+', label = 'MOOSE, 1-phase')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Liquid relative permeability")
plt.xticks([0, 0.4, 0.5, 0.6, 0.7, 0.9, 1], ["0", "0.4", "0.5", "0.6", "0.7", "0.9", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "0.8", "1"])
plt.grid()
plt.xlim([0.4, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("1-phase hysteretic relative permeability")
#plt.savefig("../../../doc/content/media/porous_flow/hys_1phase_relperm.png")

plt.figure(31)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(302)] + [0.698 + 0.001 * i for i in range(126)] + [0.824 - 0.001 * i for i in range(802)] + [0.02 + 0.001 * i for i in range(980)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "expected")
f = open("gold/1phase_relperm_2_csv.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[4] for x in data], [x[3] for x in data], 'g+', label = 'MOOSE, 1-phase')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Liquid relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "0.8", "1"])
plt.grid()
plt.xlim([0.0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("1-phase hysteretic relative permeability: cyclic saturation")
#plt.savefig("../../../doc/content/media/porous_flow/hys_1phase_2_relperm.png")

plt.figure(32)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
svals = [1 - 0.001 * i for i in range(457)] + [0.543 + 0.001 * i for i in range(457)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "exepcted, liquid")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "expected, gas")
plt.plot([1.0 - hys.s_grdel_land(0.543)], [0.0], 'gs', label = "$1 - S_{gr}^{\Delta}$")
f = open("gold/2phasePS_relperm_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[5] for x in data], [x[4] for x in data], 'g+', label = 'MOOSE, liquid')
plt.plot([x[5] for x in data], [x[3] for x in data], 'b+', label = 'MOOSE, gas')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 0.6, 0.7, 0.9, 1], ["0", "0.5", "0.6", "0.7", "0.9", "1"])
plt.grid()
plt.xlim([0.5, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("2-phase hysteretic relative permeability")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_relperm.png")

Slr = 0.4
krgmax = 0.4
relperm_gas_extension = "linear_like"
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)
plt.figure(33)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
plt.text(0, krgmax, "$k_{r,g}^{max}$", horizontalalignment='right', verticalalignment='center')
svals = [1 - 0.001 * i for i in range(725)] + [0.275 + 0.001 * i for i in range(725)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "exepcted, liquid")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "expected, gas")
f = open("gold/2phasePS_relperm_2_linear_like.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[5] for x in data], [x[4] for x in data], 'g+', label = 'MOOSE, liquid')
plt.plot([x[5] for x in data], [x[3] for x in data], 'b+', label = 'MOOSE, gas')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "", "0.6", "0.8", "1"])
plt.grid()
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("2-phase hysteretic relative permeability: linear-like extension")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_relperm_2_linear_like.png")

relperm_gas_extension = "cubic"
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)
plt.figure(34)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
plt.text(0, krgmax, "$k_{r,g}^{max}$", horizontalalignment='right', verticalalignment='center')
svals = [1 - 0.001 * i for i in range(725)] + [0.275 + 0.001 * i for i in range(725)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "exepcted, liquid")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "expected, gas")
f = open("gold/2phasePS_relperm_2_cubic.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[5] for x in data], [x[4] for x in data], 'g+', label = 'MOOSE, liquid')
plt.plot([x[5] for x in data], [x[3] for x in data], 'b+', label = 'MOOSE, gas')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "", "0.6", "0.8", "1"])
plt.grid()
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("2-phase hysteretic relative permeability: cubic extension")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_relperm_2_cubic.png")

Slr = 0.4
krgmax = 1.0
relperm_gas_extension = "linear_like"
hys = Hys(Slr, m, gamma, krgmax, upper_liquid_param, Slmin, Sgrmax, alphad, alphaw, nd, nw, absPcmax, lower_extension_type, upper_ratio, upper_extension_type, relperm_gas_extension)
plt.figure(35)
svals = [0.001 * i for i in range(1001)]
plt.plot(svals, hys.relperm_liquid_drying(svals), 'r-', label = "Drying, liquid")
plt.plot(svals, hys.relperm_gas_drying(svals), 'r:', label = "Drying, gas")
plt.plot([Slr, Slr], [0, 1], 'k:')
plt.text(Slr, -1E-2, "$S_{lr}$", horizontalalignment='center', verticalalignment='top')
plt.plot([1 - Sgrmax, 1 - Sgrmax], [0, 1.0], 'k:')
plt.text(1 - Sgrmax, -1E-2, "$1 - S_{grmax}$", horizontalalignment='center', verticalalignment='top')
plt.text(0, krgmax, "$k_{r,g}^{max}$", horizontalalignment='right', verticalalignment='center')
svals = [1 - 0.001 * i for i in range(725)] + [0.275 + 0.001 * i for i in range(725)]
kvals = hys.relperms(svals, True)
plt.plot(svals, [k[0] for k in kvals], 'g--', label = "exepcted, liquid")
plt.plot(svals, [k[1] for k in kvals], 'b--', label = "expected, gas")
f = open("gold/2phasePS_relperm_2_none.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
plt.plot([x[5] for x in data], [x[4] for x in data], 'g+', label = 'MOOSE, liquid')
plt.plot([x[5] for x in data], [x[3] for x in data], 'b+', label = 'MOOSE, gas')
plt.xlabel("Aqueous saturation ($S_{l}$)")
plt.ylabel("Relative permeability")
plt.xticks([0, 0.5, 1], ["0", "0.5", "1"])
plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1], ["0", "0.2", "0.4", "0.6", "0.8", ""])
plt.grid()
plt.xlim([0, 1])
plt.ylim([-1E-2, 1 + 1E-3])
plt.legend()
plt.tight_layout()
plt.title("2-phase hysteretic relative permeability: no extension")
#plt.savefig("../../../doc/content/media/porous_flow/hys_2phasePS_relperm_2_none.png")


plt.show()

sys.exit(0)
