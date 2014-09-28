package magneto_test

import java.lang.Thread

class TestGeneral extends Test {
    val kHostAddr = "127.0.0.1"
    val agent_plus_and_minus_port = 7627
    val agent_plus_and_plus_port = 7628
    val agent_minus_and_minus_port = 7629
    val agent_ping_port = 7630
    val plus_server0_port = 7600
    val plus_server1_port = 7602
    val minus_server0_port = 7601
    val minus_server1_port = 7603

    override def PreTest {
        for (service <- Array("agent", "service0", "service1")) {
            Public.Cmd(s"cd data/${service}; rm -rf log/*")
            Public.CmdInBackground(s"./bin/${service}", s"data/${service}")
        }
        Thread.sleep(1000)
        Config
    }

    override def Run {
        for (i <- 0 to 1000) {
            TestCase0
        }
    }

    override def PostTest {}

    def Config {
        val conf_agent_normal = "data/agent/conf/normal.conf"
        val conf_agent_service = "data/agent/conf/services.conf"
        Public.Cmd(s"sed -i '/plus_and_minus/s/:[0-9][0-9]*/:${agent_plus_and_minus_port}/g' ${conf_agent_normal}")
        Public.Cmd(s"sed -i '/plus_and_plus/s/:[0-9][0-9]*/:${agent_plus_and_plus_port}/g' ${conf_agent_normal}")
        Public.Cmd(s"sed -i '/minus_and_minus/s/:[0-9][0-9]*/:${agent_minus_and_minus_port}/g' ${conf_agent_normal}")
        Public.Cmd(s"sed -i '/ping/s/:[0-9][0-9]*/:${agent_ping_port}/g' ${conf_agent_normal}")

        Public.Cmd(s"sed -i '/plus0/s/:[0-9][0-9]*/:${plus_server0_port}/g' ${conf_agent_service}")
        Public.Cmd(s"sed -i '/plus1/s/:[0-9][0-9]*/:${plus_server1_port}/g' ${conf_agent_service}")
        Public.Cmd(s"sed -i '/minus0/s/:[0-9][0-9]*/:${minus_server0_port}/g' ${conf_agent_service}")
        Public.Cmd(s"sed -i '/minus1/s/:[0-9][0-9]*/:${minus_server1_port}/g' ${conf_agent_service}")
        val conf_service0_normal = "data/service0/conf/normal.conf"
        Public.Cmd(s"sed -i '/plus/s/:[0-9][0-9]*/:${plus_server0_port}/g' ${conf_service0_normal}")
        Public.Cmd(s"sed -i '/minus/s/:[0-9][0-9]*/:${minus_server0_port}/g' ${conf_service0_normal}")
        val conf_service1_normal = "data/service1/conf/normal.conf"
        Public.Cmd(s"sed -i '/plus/s/:[0-9][0-9]*/:${plus_server1_port}/g' ${conf_service1_normal}")
        Public.Cmd(s"sed -i '/minus/s/:[0-9][0-9]*/:${minus_server1_port}/g' ${conf_service1_normal}")
    }

    def TestCase0 {
        var result = new Array[Byte](1024)
        var ret = 0
        
        ret = SimpleTalk.Talk(kHostAddr, agent_plus_and_minus_port, Array[Byte](3), result)
        assert(9 == ret)
        assert(6 == result(ret-1))

        ret = SimpleTalk.Talk(kHostAddr, agent_plus_and_plus_port, Array[Byte](3), result)
        assert(9 == ret)
        assert(8 == result(ret-1))

        ret = SimpleTalk.Talk(kHostAddr, agent_minus_and_minus_port, Array[Byte](3), result)
        assert(9 == ret)
        assert(4 == result(ret-1))

        ret = SimpleTalk.Talk(kHostAddr, agent_ping_port, Array[Byte](2), result)
        assert('p' == result(0))
    }
}

// vim: set ts=4 sw=4 et:
