import random

from enum import Enum

from hapPy.ModeSelect import DriverBrd

class Method(Enum):
    MODE_AMP =      1
    MODE_WAVE=      2
    MODE_WAVEAMP=   3
    MODE_RANDOM=    4

class StimNorm:
    # amp range 10 - 60 %
    A_0=     0.33 #10.
    A_BAND=  0.90 - 0.33#50.#60.
    AMP=    A_0 + A_BAND
    # const amp
    AMPWAVE=A_0 + A_BAND / 2#180
    # freq mod
    FREQ_0 =    1.
    FREQ_BAND = 3. - 1.#1.5

    CLICK_MOTOR=0
    MOTORS=     [3,4,0,5,1,2]
    N_MOTORS=   len(MOTORS)
    LAST_MOTOR= N_MOTORS-1 # motors -1

    def __init__(self):
        self.brd = DriverBrd()
        #self.wb.setLRA(False)
        self.brd.setSequence(StimNorm.MOTORS)
        self.brd.setValueAll(0)
        self.brd.setEnable(True)

    def stimulate(self, level,method):
        self.brd.setValueAll(0, False)
        # S1 - amp
        if method is Method.MODE_AMP:
            self.__playClick(level)
        # S2 - wave
        elif method is Method.MODE_WAVE:
            self.__playWave(level, StimNorm.AMPWAVE)
        # S3 - freq and amp vaiations
        # S4 - S3 random
        else:
            self.__playWave(level,None)

    def stopStim(self):
        self.brd.setWave(1, 1, 0)
        #redundant:
        #self.wb.setValueAll(0)

    def prepareTrial(self,method):
        if method is Method.MODE_RANDOM:
            seq=StimNorm.MOTORS
            random.shuffle(seq)
            self.brd.setSequence(seq)
        else:
            self.brd.setSequence(StimNorm.MOTORS)

    def __playClick(self,level):
        A=int(StimNorm.A_0 + level * StimNorm.A_BAND)
        self.brd.setValue(StimNorm.CLICK_MOTOR, A)

    def __playWave(self,level,amp):
        if amp is None:
            A=int(StimNorm.A_0 + level * StimNorm.A_BAND)
        else:
            A=int(amp)
        d=1
        # 1 ms is the default toff time hardcoded in the arduino
        tOff=0.001
        freq= StimNorm.FREQ_0 + level * StimNorm.FREQ_BAND
        tau=1./freq / StimNorm.N_MOTORS # 1/f/6 [s]
        tOn = tau-tOff
        self.brd.setWave(A, tOn, d)
