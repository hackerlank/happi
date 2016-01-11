import os

__author__ = 'm'

import time
import serial


class DriverBrd:
    CMD_SETVAL	="SET"	# set PWM value in [%] "SET;2;50"
    CMD_SETALL	="SETA"	# set PWM value in [%] all motors "SETA;;50"
    CMD_INFO 	="INFO"	# get firmware info "INFO;"
    CMD_ENABLE 	="EN"	# enable motors "EN;1"
    CMD_SET_LRA ="LRA"	# toggle LRA "LRA;1"
    CMD_OK 		="OK"	# reply OK
    CMD_ERROR 	="ERR"	# reply ERROR
    
    CMD_SQ      ="SQ"	# set motor sequence "SQ;5;0,2,1,4,5"
    WAVE_2P 	="W2P"	# set on-time [ms] and amp [%] "W2P;<t_on[ms]>;<amp[%]>" ... "W2P;60;50"
    WAVE_EN 	="WEN" 	# wave direction +/-1 and 0 to disable "WEN;-1"

    """ @deprecated """
    WAVE_PARAMS ="WP"	# set all params "WP;<float>CSV" @ deprecated
    WAVE_F0 	="WF0"
    WAVE_FK 	="WFK"
    WAVE_A0 	="WA0"
    WAVE_AK 	="WAK"


    MOTORS_DEF=[0,1,2,3,4,5]

    def __init__(self, serialPort=None, seq=MOTORS_DEF):
        if serialPort is None:
            serialPort='COM4' if os.name == 'nt' else '/dev/ttyUSB0'

        print "connecting to " + serialPort + " at " + str(38400)
        self.ser = serial.Serial(timeout=1)
        self.ser.port = serialPort
        self.ser.baudrate = 38400
        self.ser.parity=serial.PARITY_NONE
        self.ser.stopbits=serial.STOPBITS_ONE
        self.ser.bytesize=serial.EIGHTBITS
        self.ser.open()
        self.seq=seq
        self.setSequence(seq)

    def close(self):
        self.ser.close()

    def __del__(self):
        self.close()


    def sendCmd(self,cmd, waitForReply=True):
        """
        private method - cmd is a byte string
        """
        tout=1
        self.ser.write(cmd+';\r')
        #print '>>'+cmd+';'
        if waitForReply:
            time.sleep(.001)
            tic = time.time()
            while self.ser.inWaiting()==0:
                el=time.time() - tic
                if el > tout:
                    raise IOError("Timeout")
            rep=""
            while self.ser.inWaiting()>0:
                rep=rep+self.ser.readline()

            return rep

        #else:
        if self.ser.inWaiting()>0:
            self.ser.flushInput()


    def setEnable(self,b):
        self.sendCmd('%s;%d'%(DriverBrd.CMD_ENABLE,b),True)

    def setLra(self,b):
        self.sendCmd('%s;%d'%(DriverBrd.CMD_SET_LRA,b),True)

    def getInfo(self):
        return self.sendCmd(DriverBrd.CMD_INFO,True)

    def setValue(self, motor, value, wait=True):
        """
        set motor voltage in PER CENT!
        """
        if value>100 or value<0:
            raise ValueError('out of bounds')
        return self.sendCmd('%s;%d;%d'%(DriverBrd.CMD_SETVAL,motor,value),wait)

    def setValueAll(self, value, wait=True):
        """
        set all motor voltages in PER CENT!
        """
        if value>100 or value<0:
            raise ValueError('out of bounds')
        #OLD
        #for motor in self.seq:
        #    self.sendCmd('%s;%d;%d'%(DriverBrd.CMD_SETVAL,motor,value),wait)
        return self.sendCmd('%s;;%d'%(DriverBrd.CMD_SETALL,value),wait)


    def setSequence(self,seq):
        cmd='%s;%i;%s'%(DriverBrd.CMD_SQ,len(seq),','.join(str(x) for x in seq))
        #print cmd
        return self.sendCmd(cmd)

    def setWave(self, amp,tOn,wdir):
        self.sendCmd('%s;%d;%d'%(DriverBrd.WAVE_2P,tOn,amp),False)
        return self.sendCmd('%s;%d'%(DriverBrd.WAVE_EN,wdir),True)