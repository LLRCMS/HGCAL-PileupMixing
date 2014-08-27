from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "Photon_140PU"
batch.hardScatterFile = "/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/Photon.root"
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasA.root")
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasB.root")
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasC.root")
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasD.root")
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/FromReducedTrees_30052014_v2/Photon_140PU/"
batch.eventsPerJob = 100
batch.nPileup = 140

batch.launch(local=False,simulate=False)
