__author__ = 'm'

from hapPy.ModeSelect import DriverBrd
import time
import sys
import argparse

class NewApiTest:
    port='/dev/ttyUSB0'
    port='COM6'
    @staticmethod
    def click():
        brd=DriverBrd(NewApiTest.port)
        #print brd.setSequence([3,4,0,5,1,2])
        print brd.getInfo()
        motor=2
        print brd.setValue(motor,20)
        time.sleep(.01)
        print brd.setValue(motor,0)

    @staticmethod
    def waveLocal():
        brd=DriverBrd(NewApiTest.port)
        time.sleep(2)
        #print brd.setSequence([3,4,0,5,1,2])
        print brd.setWave(20,100,1)
        time.sleep(10)
        print brd.setWave(35,1000,0)

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

    @staticmethod
    def info():
        brd=DriverBrd()
        print brd.getInfo()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='New API test')
    parser.add_argument('-i',   action="store_true", dest="info", default=False, help='info')
    parser.add_argument('-c',   action="store_true", dest="click", default=False, help='click')
    res=parser.parse_args()

    if res.info:
        pass#NewApiTest.info()

    if res.click:
        NewApiTest.click()