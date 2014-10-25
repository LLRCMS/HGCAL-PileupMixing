import os
import shutil
import subprocess
import stat
import sys
import random
import ROOT


class BatchJobs:
    def __init__(self):
        self.name = "fa3VsfL1"
        self.hardScatterFile = ""
        self.minBiasFiles = []
        self.tree = "hgcSimHitsAnalyzer/HGC"
        self.outpuDir = ""
        self.eventsPerJob = 100
        self.nEvents = -1
        self.nPileup = 200
        self.nJobs = 0
        self.randomSeed = 739397

        self.scram_arch = "slc6_amd64_gcc472"
        self.cmsswDir = "/home/llr/cms/sauvan/CMSSW/EmptyArea/CMSSW_5_3_5/"
        self.cwd = os.getcwd()
        self.queue = "cms"
        self.qsubArgs = []

    def createConfigs(self):
        if not os.path.exists(self.outputDir+"/jobs/"):
            os.makedirs(self.outputDir+"/jobs/")
        if not os.path.exists(self.outputDir+"/logs/"):
            os.makedirs(self.outputDir+"/logs/")
        os.symlink(os.getcwd()+"/../../obj/", self.outputDir+"/obj")
        inputFile = ROOT.TFile.Open(self.hardScatterFile)
        tree = inputFile.Get(self.tree)
        tree.__class__ = ROOT.TTree
        nEvents = tree.GetEntries()
        if self.nEvents!=-1:
            nEvents = min(nEvents, self.nEvents)
        inputFile.Close()
        self.nJobs = nEvents/self.eventsPerJob
        if nEvents%self.eventsPerJob!=0: self.nJobs += 1
        for job in range(0,self.nJobs):
            firstEntry = job*self.eventsPerJob
            lastEntry = min( (job+1)*self.eventsPerJob, nEvents)
            with open(self.outputDir+"/jobs/{0}_{1}.config".format(self.name,job), 'w') as config:
                print >>config, "HardScatterFile: {0}".format(self.hardScatterFile)
                print >>config, "MinBiasFile: {0}".format(random.choice(self.minBiasFiles))
                print >>config, "OutputFile: {0}/{1}_{2}.root".format(self.outputDir,self.name,job)
                print >>config, "Tree: {0}".format(self.tree)
                print >>config, "FirstEntry: {0}".format(firstEntry)
                print >>config, "LastEntry: {0}".format(lastEntry)
                print >>config, "NPileup: {0}".format(self.nPileup)
                print >>config, "RandomSeed: {0}".format(self.randomSeed+101*job)


    def createScripts(self):
        for job in range(0,self.nJobs):
            with open(self.outputDir+"/jobs/{0}_{1}.sub".format(self.name,job), 'w') as script:
                print >>script, "#! /bin/sh"
                print >>script, "uname -a"
                print >>script, "export SCRAM_ARCH={0}".format(self.scram_arch)
                print >>script, "source /cvmfs/cms.cern.ch/cmsset_default.sh "
                print >>script, "cd "+self.cmsswDir+"/src/"
                print >>script, "cmsenv"
                print >>script, "cd", self.outputDir
                print >>script, "\necho Executing job"
                print >>script, "export X509_USER_PROXY="+os.environ["HOME"]+"/.t3/proxy.cert"
                print >>script, self.cwd+"/../../mixing.exe","jobs/{0}_{1}.config".format(self.name,job) , "&>", "logs/{0}_{1}.log".format(self.name,job)


    def prepareCommands(self):
        for job in range(0,self.nJobs):
            qsubArgs = []
            qsubArgs.append("-k")
            qsubArgs.append("oe")
            qsubArgs.append("-N")
            qsubArgs.append(self.name+"_"+str(job))
            qsubArgs.append("-q")
            qsubArgs.append(self.queue+"@llrt3")
            qsubArgs.append("-v")
            qsubArgs.append("X509_USER_PROXY="+os.environ["HOME"]+"/.t3/proxy.cert")
            qsubArgs.append("-V")
            qsubArgs.append("{0}/jobs/{1}_{2}.sub".format(self.outputDir,self.name,job))
            self.qsubArgs.append(qsubArgs)

    def launch(self, local=False, simulate=False):
        self.createConfigs()
        self.createScripts()
        self.prepareCommands()
        for qsubArgs in self.qsubArgs:
            if not local:
                command = "qsub"
                for arg in qsubArgs:
                    command += " "+arg
                print command
                if not simulate:
                    subprocess.call(["qsub"] + qsubArgs)
            else:
                if not simulate:
                    os.chmod("{0}/jobs/{0}_{1}.sub".format(self.outputDir,self.name,job),stat.S_IRWXU)
                    subprocess.call(["{0}/jobs/{0}_{1}.sub".format(self.outputDir,self.name,job)])

