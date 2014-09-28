package magneto_test

object Runall {
    val cases = Array(new TestGeneral)

    def RunTests {
        Test.PreGlobal
        for (testcase <- cases) {
            testcase.PreTest
            testcase.Run
            testcase.PostTest
        }
        Test.PostGlobal
    }

    def main(args :Array[String]) {
        RunTests
    }
}

// vim: set ts=4 sw=4 et:
