from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "MinBias_140PU"
batch.cmsswDir = "/home/llr/cms/sauvan/CMSSW/HGCAL/CMSSW_6_2_0_SLHC20/"
batch.hardScatterFile = "/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_14.11.27/MinBias_0.root"
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_14.11.27/MinBias_0.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_14.11.27/MinBias_1.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC20_14.11.27/MinBias_2.root")
batch.tree = "ntuplizer/HGC"
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/MinBias_140PU/6_2_0_SLHC20_14.11.27/"
batch.eventsPerJob = 100
batch.nEvents = 5000
batch.nPileup = 140
batch.randomSeed = 73939

batch.launch(local=False,simulate=False)
