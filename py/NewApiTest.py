__author__ = 'm'

from hapPy.ModeSelect import DriverBrd
import time
import sys

class NewApiTest:
    port='/dev/ttyUSB0'
    port='COM4'
    @staticmethod
    def click():
        brd=DriverBrd(NewApiTest.port)
        print brd.setSequence([3,4,0,5,1,2])
        print brd.getInfo()
        motor=2
        print brd.setValue(motor,10)
        time.sleep(1)
        #print brd.setValue(motor,20)
        time.sleep(1)
        print brd.setValue(motor,0)

    @staticmethod
    def waveLocal():
        brd=DriverBrd()
        print brd.setSequence([3,4,0,5,1,2])
        print brd.setWave(55,400,-1)
        time.sleep(5)
        print brd.setWave(35,400,0)

    @staticmethod
    def diag():
        brd=DriverBrd(NewApiTest.port)
        print brd.setValue(0,10)
        time.sleep(.1)
        print brd.setValue(3,30)
        time.sleep(.1)
        print brd.setValue(0,0)
        time.sleep(.2)
        print brd.setValue(3,0)

    @staticmethod
    def close():
        brd=DriverBrd()
        brd.close()
        brd=DriverBrd()
        print brd.setSequence([3,4,0,5,1,2])
        print brd.getInfo()

def main(argv):
    #NewApiTest.click()
    NewApiTest.waveLocal()

if __name__ == "__main__":
    main(sys.argv)