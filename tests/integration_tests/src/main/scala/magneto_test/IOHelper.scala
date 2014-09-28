package magneto_test

import java.lang.Object
import java.lang.Throwable
import java.net.Socket
import java.io.{DataOutputStream,DataInputStream,OutputStreamWriter,InputStreamReader}

object SimpleTalk {
    val kMagic = 58764 toShort
    val kVersion = 0 toShort

    def Talk(host :String, port :Int, out :Array[Byte], in :Array[Byte]) :Int ={
        val socket = new Socket(host, port)
        socket.setSoTimeout(1000)
        val out_buf = new DataOutputStream(socket getOutputStream)
        out_buf.writeInt(out size)
        out_buf.writeShort(kMagic)
        out_buf.writeShort(kVersion)
        out_buf.write(out, 0, out size)
        out_buf.flush

        val in_buf = new DataInputStream(socket getInputStream)
        val len_buf = in_buf.read(in)

        out_buf.close
        in_buf.close
        socket.close
        len_buf
    }
}

// vim: set ts=4 sw=4 et:
