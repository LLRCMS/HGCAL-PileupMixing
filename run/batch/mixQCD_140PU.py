from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "QCD_14"
batch.cmsswDir = "/home/llr/cms/sauvan/CMSSW/HGCAL/CMSSW_6_2_0_SLHC20/"
batch.hardScatterFile = "/data_CMS/cms/sauvan/HGCAL/QCD/CMSSW_6_2_0_SLHC20_15.01.03/QCD_14.root"
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_4.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_5.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_6.root")
batch.tree = "ntuplizer/HGC"
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/QCD_140PU/CMSSW_6_2_0_SLHC20_15.01.03/14/"
batch.eventsPerJob = 300
batch.nPileup = 140
#batch.randomSeed = 84110 # 0
#batch.randomSeed = 59550 # 1
#batch.randomSeed = 94789 # 2
#batch.randomSeed = 83675 # 3
#batch.randomSeed = 38926 # 4
#batch.randomSeed = 76464 # 5
#batch.randomSeed = 57054 # 6
#batch.randomSeed = 7357  # 7
#batch.randomSeed = 88122 # 8
#batch.randomSeed = 60230 # 9
#batch.randomSeed = 47448 # 10
#batch.randomSeed = 20344 # 11
#batch.randomSeed = 31397 # 12
#batch.randomSeed = 35367 # 13
batch.randomSeed = 53698 # 14
batch.hitEnergyThreshold = 1.

batch.launch(local=False,simulate=False)
