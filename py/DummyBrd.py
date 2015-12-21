from hapPy.DriverBrd import DriverBrd


class DummyBrd(DriverBrd):

    def __init__(self, serialPort=None, seq=DriverBrd.MOTORS_DEF):
        print "connecting to Dummy"
        self.seq=seq

    def close(self):
        pass

    def __del__(self):
        pass

    def sendCmd(self, cmd, waitForReply=True):
        print cmd
        pass
