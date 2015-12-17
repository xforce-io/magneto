import os

kRootDir = "../../"
kExampleDir = "../../build/examples/"

def testExample() :
    os.system("mkdir -p %s/build" % kRootDir)
    os.system("cd %s/build && cmake ../ && make" % kRootDir)

def test() :
    testExample()
