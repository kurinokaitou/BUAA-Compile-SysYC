import subprocess

INTERMEDIATE_DIR = "./intermediate/"
class Runner:

    def __init__(self, testPath, outputPath, inputPath = None, mars = False, llvm = False):
        self.testPath = testPath # 测试文件路径
        self.outputPath = outputPath # 答案路径
        self.inputPath = inputPath # 输入文件路径
        self.mars = mars
        self.llvm = llvm

    def copyFile(self, source, destination):
        sourceFile = open(source, "r", encoding="utf-8")
        contents = sourceFile.readlines()
        sourceFile.close()
        destFile = open(destination, "w")
        for s in contents:
            destFile.write(s)
        destFile.close()    


    def writeToTestFile(self):
        # 将输入写入testfile
        self.copyFile(source=self.testPath, destination= INTERMEDIATE_DIR+"testfile.txt")

    def writeToOutputFile(self):
        # 将答案写入ansfile
        self.copyFile(source=self.outputPath, destination=INTERMEDIATE_DIR+"output.txt") 

    def writeToInputFile(self):
        if self.inputPath != None:
            self.copyFile(source=self.inputPath, destination=INTERMEDIATE_DIR+"input.txt")

    def check(self):
        # 比较ansfile和outputfile
        resultFile = open(INTERMEDIATE_DIR+"result.txt","r")
        outputFile = open(INTERMEDIATE_DIR+"output.txt","r")
        ans = resultFile.readlines()
        outputs = outputFile.readlines()
        if len(ans) != len(outputs):
            return False
        for i in range(0,len(ans)):
            if ans[i].split() != outputs[i].split():
                return False
        return True                  

    def getStdInput(self):
        if self.inputPath == None:
            self.stdInput = ""
            return self.stdInput

        inputFile = open(INTERMEDIATE_DIR+"input.txt","r")
        inputs = inputFile.readlines()
        self.stdInput = ""
        for s in inputs:
            self.stdInput += s
        return self.stdInput    

    def runMars(self):
        cmd = ['java','-jar','mars.jar',INTERMEDIATE_DIR+'mips.txt', 'nc']
        cwd = './'
        process = subprocess.Popen(args=cmd,
                                    cwd=cwd,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    stdin=subprocess.PIPE)

        stdout = process.communicate(input=str.encode(self.getStdInput()))
        f = open(INTERMEDIATE_DIR+'result.txt','w')
        for b in stdout:
            f.write(bytes.decode(b))
        f.close()    
    
    def runLLVM(self):
        cmd = ['lli', INTERMEDIATE_DIR+'llvm_ir.txt']
        cwd = './'
        process = subprocess.Popen(args=cmd,
                                    cwd=cwd,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    stdin=subprocess.PIPE)

        stdout = process.communicate(input=str.encode(self.getStdInput()))
        f = open(INTERMEDIATE_DIR+'result.txt','w')
        for b in stdout:
            f.write(bytes.decode(b))
        f.close()    
                                                      

    def run(self):
        ## read input
        self.writeToTestFile()
        self.writeToOutputFile()
        self.writeToInputFile()
        cmd = ['../../dist/sysyc', '--dump-ir', 'llvm_ir.txt', '--dump-mips', 'mips.txt' 'testfile.txt']   
        cwd = INTERMEDIATE_DIR
        process = subprocess.run(args=cmd,
                                    cwd=cwd,
                                    stdout=subprocess.PIPE,
                                    #stderr=subprocess.PIPE,
                                    stdin=subprocess.PIPE)
        #process.communicate(input=str.encode(self.getStdInput()))
        if self.mars:
            self.runMars()
        if self.llvm:
            self.runLLVM()
     







