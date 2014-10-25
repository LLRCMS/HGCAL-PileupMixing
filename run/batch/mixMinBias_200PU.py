from BatchJobs import BatchJobs


batch = BatchJobs()
batch.name = "MinBias_200PU"
batch.hardScatterFile = "/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasA.root"
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasB.root")
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasC.root")
batch.minBiasFiles.append("/data_CMS/cms/salerno/HGC/ReducedTrees_30052014/MinBiasD.root")
batch.outputDir = "/data_CMS/cms/sauvan/HGCAL/FromReducedTrees_30052014_v2/MinBias_200PU/"
batch.eventsPerJob = 100
batch.nEvents = 10000
batch.nPileup = 200
batch.randomSeed =  65537

batch.launch(local=False,simulate=False)
