package magneto_test

import sys.process._

object Public {
    def Cmd(cmd :String, ignore :Boolean=false) {
        val ret = Seq("sh", "-c", cmd) !;
        if (0!=ret && !ignore) {
            throw new RuntimeException("fail_execute[%s]" format cmd)
        }
    }

    def CmdInBackground(cmd :String, dir :String=".", ignore :Boolean=false) {
        Public.Cmd(s"cd ${dir} && echo '${cmd} &> /dev/null &' > .tmp.sh && sh .tmp.sh", ignore)
    }
}

// vim: set ts=4 sw=4 et:
