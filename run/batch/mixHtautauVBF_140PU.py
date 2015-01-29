from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "VBF_Htautau_3"
batch.cmsswDir = "/home/llr/cms/sauvan/CMSSW/HGCAL/CMSSW_6_2_0_SLHC20/"
batch.hardScatterFile = "/data_CMS/cms/sauvan/HGCAL/VBF_Htautau/CMSSW_6_2_0_SLHC20_15.01.03/VBF_Htautau_3.root"
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_3.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_4.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_15.01.03/MinBias_5.root")
batch.tree = "ntuplizer/HGC"
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/VBF_Htautau_140PU/CMSSW_6_2_0_SLHC20_15.01.07/3/"
batch.eventsPerJob = 300
batch.nPileup = 140
#batch.randomSeed = 7067 # 0
#batch.randomSeed = 496  # 1
#batch.randomSeed = 6297 # 2
batch.randomSeed = 3652  # 3
batch.hitEnergyThreshold = 1.

batch.launch(local=False,simulate=False)
