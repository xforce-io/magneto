name := "magneto_test"

version := "1.0.0"

scalaVersion := "2.11.2"

libraryDependencies ++= Seq(
    "org.scalatest" % "scalatest_2.11" % "2.2.2",
    "org.json" % "json" % "20140107",
    "com.typesafe.akka" %% "akka-actor" % "2.3.4",
    "io.netty" % "netty-all" % "5.0.0.Alpha1"
    )
