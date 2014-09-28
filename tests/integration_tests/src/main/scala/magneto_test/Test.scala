package magneto_test

abstract class Test {
    def PreTest
    def PostTest
    def Run
}

object Test {
    def PreGlobal {
        Clear
       
        val resources_dir = Conf.kResourcesDir
        Public.Cmd("cd ../../ && make clean && make -j4 -s")
        Public.Cmd("mkdir -p data/{agent,service0,service1}/{bin,log,conf}")
        for (service <- Array("agent", "service0", "service1")) {
            Public.Cmd(s"cd ${resources_dir}/${service}/ && make clean && make -j4 -s")
            Public.Cmd(s"cp ${resources_dir}/${service}/bin/${service} data/${service}/bin")
            Public.Cmd(s"cp ${resources_dir}/${service}/conf/* data/${service}/conf/")
        }
    }

    def PostGlobal {
        Clear
    }

    private def Clear {
        for (service <- Array("agent", "service0", "service1")) {
            Public.Cmd(s"killall -9 ${service}", true)
        }
    }
}

// vim: set ts=4 sw=4 et:
