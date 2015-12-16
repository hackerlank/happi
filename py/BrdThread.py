import time

from hapPy.ModeSelect import DriverBrd
#from WbPyUtils import WbPyUtils
import threading


class BrdThread(threading.Thread):
    """
    abstract class, extend it and override the while loop
    """
    def __init__(self, com = None):
        self.wb = DriverBrd(com)
        self.__shouldRun = True
        #print "thread constructed"
        threading.Thread.__init__(self)
        
    def cancel(self):
        """
        call from main
        """
        self.__shouldRun=False
        #print "thread canceled"

    def getWb(self):
        """
        for internal use
        """
        return self.wb

    def shouldRun(self):
        """
        for internal use
        """
        return self.__shouldRun

    def whileLoop(self):
        """
        override this
        """
        while self.shouldRun():
            """ example """
            # t=time.time() - self.tic
            # self.getWb().setValue(1,30)
            # time.sleep(.001)
            pass
    
    def __onFinish(self):
        """
        private, don't call this
        """
        time.sleep(.005)
        self.wb.setEnable(False)
        self.wb.close()

    def run(self):
        """
        don't call this, use thread.start()
        """
        try:
            self.wb.setEnable(True)
            self.wb.setValueAll(0)
            self.tic = time.time()
            #omega=100 #degrees/s
            self.whileLoop()
        
        except KeyboardInterrupt:
            print "Caught KeyboardInterrupt, terminating workers"
        
        except AttributeError as e:
            print "attr "+e.message
        
        except IOError as e:
            print "IO "+e.message
        
        except BaseException as e:
            print "Base "+e.message
        
        #print "thread finished"
        self.__onFinish()