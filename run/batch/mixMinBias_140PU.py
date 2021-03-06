from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "MinBias_140PU_0"
batch.cmsswDir = "/home/llr/cms/sauvan/CMSSW/HGCAL/CMSSW_6_2_0_SLHC20/"
batch.hardScatterFile = "/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_0.root"
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_1.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_2.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_3.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_4.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_5.root")
batch.tree = "ntuplizer/HGC"
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/MinBias_140PU/6_2_0_SLHC20_15.01.17/0/"
batch.eventsPerJob = 300
batch.nPileup = 140
batch.randomSeed = 77569 # 0
#batch.randomSeed = 66121 # 1
#batch.randomSeed = 19997 # 2
#batch.randomSeed = 49871 # 3

batch.hitEnergyThreshold = 1.

batch.launch(local=False,simulate=False)
