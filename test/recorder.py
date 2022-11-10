import datetime
from time import strftime
class Recorder:
    def __init__(self):
        self.time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.results = dict()
        self.result = True

    def addResult(self, id, result):
        self.results[id] = result
        self.result &= result

    def addTestTime(self, time):
        self.testTime = time

    def writeToLog(self):
        f = open("./testlog/"+"testlog "+ self.time +".txt","w")
        s = "test time: "+self.time
        f.write(s +'\n')
        print(s)
        s = "test result: "
        f.write(s)
        if self.result:
            s += "\033[1;32;40mAccepted\033[0m"
            f.write("Accepted\n")
        else:
            s += "\033[1;31;40mWrong Answer\033[0m"
            f.write("Wrong Answer\n")
        print(s)
        s = "details:"
        f.write(s+'\n')
        print(s)
        for id in self.results.keys():
            s = "testfile"+ str(id) + "   "
            if self.results[id]:
                s += "Accepted"
            else:
                s += "Wrong Answer"
            f.write(s+'\n')  
            print(s)
        f.write('\n')    
        s = "total test time: " + str(self.testTime) + "ms\n"
        print(s)
        f.write(s);
        f.close()                 