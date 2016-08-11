import random

from enum import Enum

from hapPy.ModeSelect import DriverBrd
from hapPy.Stimuli import Method
from hapPy.StimNorm import StimNorm

class StimT(StimNorm):
    TAU_0=50
    # TAU=TON+TOFF at f=1Hz and n=6 and T_OFF=0
    TAU_MAX=(1./6.)
    def __playWave(self,level,amp):
        if amp is None:
            A=(StimNorm.A_0 + level * StimNorm.A_BAND)
        else:
            A= amp
        d=1
        # 1 ms is the default toff time hardcoded in the arduino
        tOff=0.001
        #freq= StimNorm.FREQ_0 + level * StimNorm.FREQ_BAND
        #tau=1./freq / StimNorm.N_MOTORS # 1/f/6 [s]
        tau=StimT.TAU_0+(1-level)*(StimT.TAU_MAX - StimT.TAU_0)

        tOn = tau-tOff
        #print 'ton=%f amp=%f'%(tOn,A)
        self.brd.setWave(100*A, 1000*tOn, d)
