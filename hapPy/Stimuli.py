import random

from enum import Enum

from hapPy.ModeSelect import DriverBrd

class Method(Enum):
    MODE_AMP=1
    MODE_WAVE=2
    MODE_WAVEAMP=3
    MODE_RANDOM=4

class Stimuli:
    # amp range 10 - 60 %
    A0=33 #10.
    Arange=90-33#50.#60.
    AMP=A0+Arange
    # const amp
    AMPWAVE=A0+Arange/2#180
    # freq mod
    FREQ_RANGE = 2.5#1.5
    FREQ_0 = 1.

    CLICK_MOTOR=0
    MOTORS=[3,4,0,5,1,2]
    LAST_MOTOR=len(MOTORS)-1 # motors -1
    N_MOTORS=len(MOTORS)

    def __init__(self):
        self.wb = DriverBrd()
        #self.wb.setLRA(False)
        self.wb.setSequence(Stimuli.MOTORS)
        self.wb.setValueAll(0)
        self.wb.setEnable(True)

    def stimulate(self, level,method):
        self.wb.setValueAll(0,False)

        # S1 - amp
        if method is Method.MODE_AMP:
            self.__playClick(level)
        # S2 - wave
        elif method is Method.MODE_WAVE:
            self.__playWave(1.*level, Stimuli.AMPWAVE)
        # S3 - freq and amp vaiations
        # S4 - S3 random
        else:
            self.__playWave(1.*level,None)

    def stopStim(self):
        self.wb.setWave(1,1,0)
        self.wb.setValueAll(0)

    def prepareTrial(self,method):
        if method is Method.MODE_RANDOM:
            seq=Stimuli.MOTORS
            random.shuffle(seq)
            self.wb.setSequence(seq)
        else:
            self.wb.setSequence(Stimuli.MOTORS)

    def __playClick(self,level):
        A=int(Stimuli.A0+level/10.*Stimuli.Arange)
        self.wb.setValue(Stimuli.CLICK_MOTOR, A)

    def __playWave(self,level,amp):
        if amp is None:
            A=int(Stimuli.A0+level/10.*Stimuli.Arange)
        else:
            A=int(amp)
        d=1
        # 1 ms is the default toff time hardcoded in the arduino
        tOff=0.001
        freq= Stimuli.FREQ_0 + level / 10. * Stimuli.FREQ_RANGE
        tau=1./freq/Stimuli.N_MOTORS # 1/f/6 [s]
        tOn = tau-tOff
        self.wb.setWave(A,tOn,d)
