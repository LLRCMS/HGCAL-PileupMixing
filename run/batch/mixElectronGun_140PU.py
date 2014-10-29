from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "Electron_140PU"
batch.cmsswDir = "/home/llr/cms/sauvan/CMSSW/HGCAL/CMSSW_6_2_0_SLHC19/"
batch.hardScatterFile = "/data_CMS/cms/sauvan/HGCAL/ElectronGun/CMSSW_6_2_0_SLHC19_14.10.28/ElectronGun_PU0.root"
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC19_14.10.28/MinBias_1.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC19_14.10.28/MinBias_2.root")
batch.minBiasFiles.append("/data_CMS/cms/sauvan/HGCAL/MinBias/6_2_0_SLHC19_14.10.28/MinBias_3.root")
batch.tree = "ntuplizer/HGC"
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/ElectronGun_140PU/CMSSW_6_2_0_SLHC19_14.10.28/"
batch.eventsPerJob = 200
batch.nPileup = 140
batch.randomSeed = 739397

batch.launch(local=False,simulate=False)
