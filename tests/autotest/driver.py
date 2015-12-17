import sys
import env.pressure.run_cases

reload(sys)
sys.setdefaultencoding("utf-8")

if __name__ == "__main__" :
    if len(sys.argv) < 2 or (sys.argv[1] != "pressure") :
        print "Usage : %s [pressure]" % (sys.argv[0])      
        sys.exit(1)

    if sys.argv[1] == "pressure" :
        env.pressure.run_cases.runCases(sys.argv[2:])
