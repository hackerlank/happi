package hapbrdlib;

import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * VibroBrd implements communication using the original 2 Byte firmware
 */
public class HapBrd extends Thread {
    private final BluetoothSocket mmSocket;
    private final InputStream mmInStream;
    protected final OutputStream mmOutStream;
    private List<WbBtListener> listeners=new ArrayList<>();

    public HapBrd(BluetoothSocket socket) {
        mmSocket = socket;
        InputStream tmpIn = null;
        OutputStream tmpOut = null;

        // Get the input and output streams, using temp objects because
        // member streams are final
        try {
            tmpIn = socket.getInputStream();
            tmpOut = socket.getOutputStream();
        } catch (IOException e) { }

        mmInStream = tmpIn;
        mmOutStream = tmpOut;

        Log.d("wb", "constructed");
    }


    public void run() {
        byte[] buffer = new byte[128];  // buffer store for the stream
        int bytes; // bytes returned from read()

        // Keep listening to the InputStream until an exception occurs
        while (true) {
            try {
                // Read from the InputStream
                bytes = mmInStream.read(buffer);
                // Send the obtained bytes to the UI activity
                //mHandler.obtainMessage(MESSAGE_READ, bytes, -1, buffer)
                //        .sendToTarget();
                String str = new String(buffer, "US-ASCII").substring(0, bytes);
                Log.d("wb","rec: len="+ bytes + " txt=" +
                        str);
                for (WbBtListener l : listeners) {
                    l.wbResponded(str);
                }
            } catch (IOException e) {
                break;
            }
        }
    }

	/* Call this from the main activity to shutdown the connection */
    public void cancel() {
        try {
            mmSocket.close();
        } catch (IOException e) { }
        for (WbBtListener l : listeners) {
            l.connectionLost();
        }
    }
	
    /* internal */
    protected void writeBytes(byte[] bytes) throws IOException{
        mmOutStream.write(bytes);
        Log.d("wb","sent: "+bytes[0]+" "+bytes[1] +" = "+new String(bytes, "US-ASCII"));
    }

    /* enable drivers */
    public void setEnabled(boolean e) throws IOException{
        Log.d("wb","enable "+e);
		writeBytes(String.format("EN;%d;\r",e).getBytes("US-ASCII"));
    }

	public void setLRA(boolean value) throws IOException {
        writeBytes(String.format("LRA;%d;\r",value).getBytes("US-ASCII"));
    }
	
    /* set individual motor power rate [0 1] */
    public void setMotorPercentage(int motor, double value) throws IOException {
        writeBytes(String.format("SET;%d;%d;\r",motor,value*100).getBytes("US-ASCII"));
    }
    

	public void requestInfo() throws IOException {
        writeBytes(String.format("INFO;%d;\r",true).getBytes("US-ASCII"));
    }
    public void setSequence(byte [] seq) throws IOException {
        String cmd="SQ;";
        for (byte b : seq) {
           cmd+=b+",";
        }
        cmd+=";\r";
        writeBytes(cmd.getBytes("US-ASCII"));
    }
    public void setWave(double amp, int tOn, int dir) throws IOException{
        writeBytes(String.format("W2P;%d;%d;\r",amp*100,tOn).getBytes("US-ASCII"));
        writeBytes(String.format("WEN;%d;\r",dir).getBytes("US-ASCII"));
    }
	
    public void setListener(WbBtListener l) {
        listeners.add(l);
    }

    public void rmListener(WbBtListener l) {
        listeners.remove(l);
    }
    public interface WbBtListener {
        /**
         * These methods are called from the background thread. If the listener
         * writes to the GUI, remember to use a scheduler!
         * @param msg
         */
        void wbResponded(String msg);
        void connectionLost();
    }
}