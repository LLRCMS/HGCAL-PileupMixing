/**
 *  @file  Mixer.cpp
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/04/2014
 *
 *  @internal
 *     Created :  06/04/2014
 * Last update :  06/04/2014 01:53:03 PM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */



#include <iostream>
#include <sstream>
#include <sys/stat.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TChain.h>
#include <TRandom3.h>

#include "Mixer.h"
#include "Utilities.h"

using namespace PileupMixing;
using namespace std;

/*****************************************************************/
Mixer::Mixer():
    m_outputFileName("output.root")
/*****************************************************************/
{
}


/*****************************************************************/
Mixer::~Mixer()
/*****************************************************************/
{
    m_outputTree->Write();
    m_outputFile->Close();
    m_hardScatterChain->Delete();
    m_minBiasChain->Delete();
}


/*****************************************************************/
bool Mixer::initialize(const std::string& parameterFile)
/*****************************************************************/
{
    // read configuration
    m_reader.read(parameterFile);

    m_outputFileName = m_reader.outputFile();

    m_hardScatterChain = new TChain(m_reader.tree().c_str());
    m_minBiasChain = new TChain(m_reader.tree().c_str());
    m_hardScatterChain->Add(m_reader.hardScatterFile().c_str());
    m_minBiasChain->Add(m_reader.minBiasFile().c_str());

    m_outputFile = TFile::Open(m_outputFileName.c_str(), "RECREATE");
    if(!m_outputFile)
    {
        cout<<"ERROR: Cannot open output file for writing\n";
        return false;
    }

    // Get input entries
    m_nEntriesHardScatter = m_hardScatterChain->GetEntries();
    m_nEntriesMinBias     = m_minBiasChain->GetEntries();

    // Initialize random generator
    int randomSeed = m_reader.randomSeed();
    m_random.SetSeed(randomSeed);

    // Branch input and output trees
    vector<string> tokens;
    tokenize(m_reader.tree(), tokens, "/");
    string outTreeName = tokens[tokens.size()-1];
    m_outputFile->cd();
    m_outputTree = new TTree(outTreeName.c_str(), outTreeName.c_str());
    branch();
    clean();

    return true;
}


/*****************************************************************/
void Mixer::loop()
/*****************************************************************/
{
    Long64_t firstEntry = max((Long64_t)0,(Long64_t)m_reader.firstEntry());
    Long64_t lastEntry  = min(m_nEntriesHardScatter, (Long64_t)m_reader.lastEntry());
    if(firstEntry>m_nEntriesHardScatter) firstEntry = m_nEntriesHardScatter-1;
    if(lastEntry<0) lastEntry = m_nEntriesHardScatter;
    if(firstEntry>lastEntry) 
    {
        Long64_t tmp = firstEntry;
        firstEntry = lastEntry;
        lastEntry = tmp;
    }
    for (Long64_t entry=firstEntry;entry<lastEntry;entry++)
    {
        //if(entry%100==0)
        //{
            cout<<"INFO: Getting hard scatter entry "<<entry-firstEntry+1<<"/"<<(lastEntry-firstEntry)<<"\n";
        //}
        m_mixedEvents.insert(entry);
        m_hardScatterChain->GetEntry(entry);
        if(m_hardScatterEvent.nhits>=MAXHIT)
        {
            stringstream error;
            error << "ERROR: number of hits in hard scatter event larger than MAXHIT ("<<MAXHIT<<")";
            throw string(error.str());
        }

        overlapHits(m_hardScatterEvent);
        overlapGen(m_hardScatterEvent);
        mix();
        fill();
        clean();
    }
}

/*****************************************************************/
void Mixer::mix()
/*****************************************************************/
{
    int nPileup = m_random.Poisson(m_reader.nPileup());
    m_mixedEvent.nPileup = nPileup;
    for (Long64_t evt=0;evt<nPileup;evt++)
    {
        Long64_t mbEvent = (Long64_t)m_random.Integer(m_nEntriesMinBias);
        // Make sure we don't double count one given event
        while( m_mixedEvents.find(mbEvent)!=m_mixedEvents.end() )
        {
            mbEvent = (Long64_t)m_random.Integer(m_nEntriesMinBias);
        }
        m_mixedEvents.insert(mbEvent);
        m_minBiasChain->GetEntry(mbEvent);
        if(m_minBiasEvent.nhits>=MAXHIT)
        {
            stringstream error;
            error << "ERROR: number of hits in minbias event larger than MAXHIT ("<<MAXHIT<<")";
            throw string(error.str());
        }
        overlapHits(m_minBiasEvent);
    }
}


/*****************************************************************/
void Mixer::overlapHits(HGCSimEvent& event)
/*****************************************************************/
{
    int nHits = event.nhits;
    for(int h=0; h<nHits; h++)
    {
        int detid = 0;
        detid |= event.hit_bin[h]; // 13 bits
        detid |= (event.hit_sec[h]<<13); // 6 bits
        int layer = (event.hit_layer[h]>=0 ? event.hit_layer[h] : event.hit_layer[h]+63);
        detid |= (layer<<19); // 5 bits
        detid |= (event.hit_type[h]<<24); // 2 bits
        auto itrHit = m_hits.find(detid);
        if(itrHit==m_hits.end())
        {
            // create new hit
            HGCSimHit hit;
            hit.type  = event.hit_type[h];
            hit.layer = event.hit_layer[h];
            hit.sec   = event.hit_sec[h];
            hit.bin   = event.hit_bin[h];
            hit.detid = detid;
            hit.edep  = event.hit_edep[h];
            hit.avgt  = event.hit_avgt[h];
            hit.x     = event.hit_x[h];
            hit.y     = event.hit_y[h];
            hit.z     = event.hit_z[h];
            hit.eta   = event.hit_eta[h];
            hit.phi   = event.hit_phi[h];
            m_hits[detid] = hit;
        }
        else
        {

            // sum hit energy. 
            // FIXME: What to do with avgt??
            HGCSimHit& hit = itrHit->second;
            //cerr<<"Summing hit energy for 0x"<<hex<<detid<<"\n";
            //cerr<<" layer="<<hit.layer<<",sec="<<hit.sec<<",bin="<<hit.bin<<"\n";
            //cerr<<" E="<<hit.edep<<"\n";
            hit.edep += event.hit_edep[h];
            //cerr<<" -> E="<<hit.edep<<"\n";
        }
    }
}

/*****************************************************************/
void Mixer::overlapGen(HGCSimEvent& event)
/*****************************************************************/
{
    int nGen = event.ngen;
    for(int g=0; g<nGen; g++)
    {
        // create new gen
        HGCSimGen gen;
        gen.id     = event.gen_id[g];
        gen.status = event.gen_status[g];
        gen.pt     = event.gen_pt[g];
        gen.eta    = event.gen_eta[g];
        gen.phi    = event.gen_phi[g];
        gen.en     = event.gen_en[g];
        m_gens.push_back(gen);
    }
}


/*****************************************************************/
void Mixer::fill()
/*****************************************************************/
{
    m_mixedEvent.run   = m_hardScatterEvent.run;
    m_mixedEvent.event = m_hardScatterEvent.event;
    m_mixedEvent.lumi  = m_hardScatterEvent.lumi;
    m_mixedEvent.nhits = 0;
    m_mixedEvent.ngen  = 0;
    if((int)m_hits.size()>=MAXHIT)
    {
        stringstream error;
        error << "ERROR: number of output hits larger than MAXHIT ("<<m_hits.size()<<")";
        throw string(error.str());
    }
    for(auto itrHit=m_hits.begin(); itrHit!=m_hits.end(); ++itrHit)
    {
        const HGCSimHit& hit = itrHit->second;
        int n = m_mixedEvent.nhits;

        m_mixedEvent.nhits++;
        m_mixedEvent.hit_type[n]  = hit.type;
        m_mixedEvent.hit_sec[n]   = hit.sec;
        m_mixedEvent.hit_layer[n] = hit.layer;
        m_mixedEvent.hit_bin[n]   = hit.bin;
        m_mixedEvent.hit_detid[n] = hit.detid;
        m_mixedEvent.hit_edep[n]  = hit.edep;
        m_mixedEvent.hit_avgt[n]  = hit.avgt;
        m_mixedEvent.hit_x[n]     = hit.x;
        m_mixedEvent.hit_y[n]     = hit.y;
        m_mixedEvent.hit_z[n]     = hit.z;
        m_mixedEvent.hit_eta[n]   = hit.eta;
        m_mixedEvent.hit_phi[n]   = hit.phi;
    }
    if((int)m_gens.size()>=MAXGEN)
    {
        stringstream error;
        error << "ERROR: number of output gen particles larger than MAXGEN ("<<m_gens.size()<<")";
        throw string(error.str());
    }
    for(auto itrGen=m_gens.begin(); itrGen!=m_gens.end(); ++itrGen)
    {
        const HGCSimGen& gen = *itrGen;
        int n = m_mixedEvent.ngen;
        m_mixedEvent.ngen++;
        m_mixedEvent.gen_id[n]     = gen.id;
        m_mixedEvent.gen_status[n] = gen.status;
        m_mixedEvent.gen_pt[n]     = gen.pt;
        m_mixedEvent.gen_eta[n]    = gen.eta;
        m_mixedEvent.gen_phi[n]    = gen.phi;
        m_mixedEvent.gen_en[n]     = gen.en;
    }
    m_outputTree->Fill();
}


/*****************************************************************/
void Mixer::clean()
/*****************************************************************/
{
    m_mixedEvents.clear();
    m_hits.clear();
    m_gens.clear();
    m_mixedEvent.event = 0;
    m_mixedEvent.lumi  = 0;
    m_mixedEvent.run   = 0;
    m_mixedEvent.ngen  = 0;
    m_mixedEvent.nhits = 0;
    for(int n=0;n<MAXGEN;n++)
    {
        m_mixedEvent.gen_id[n]     = 0;
        m_mixedEvent.gen_status[n] = 0;
        m_mixedEvent.gen_pt[n]     = 0;
        m_mixedEvent.gen_eta[n]    = 0;
        m_mixedEvent.gen_phi[n]    = 0;
        m_mixedEvent.gen_en[n]     = 0;
    }
    for(int n=0;n<MAXHIT;n++)
    {
        m_mixedEvent.hit_type[n]  = 0;
        m_mixedEvent.hit_sec[n]   = 0;
        m_mixedEvent.hit_layer[n] = 0;
        m_mixedEvent.hit_bin[n]   = 0;
        m_mixedEvent.hit_detid[n] = 0;
        m_mixedEvent.hit_edep[n]  = 0;
        m_mixedEvent.hit_avgt[n]  = 0;
        m_mixedEvent.hit_x[n]     = 0;
        m_mixedEvent.hit_y[n]     = 0;
        m_mixedEvent.hit_z[n]     = 0;
        m_mixedEvent.hit_eta[n]   = 0;
        m_mixedEvent.hit_phi[n]   = 0;
    }
}


/*****************************************************************/
void Mixer::branch()
/*****************************************************************/
{
    // Hard scatter tree
    m_hardScatterChain->SetBranchAddress("run"       , &m_hardScatterEvent.run);
    m_hardScatterChain->SetBranchAddress("lumi"      , &m_hardScatterEvent.lumi);
    m_hardScatterChain->SetBranchAddress("event"     , &m_hardScatterEvent.event);

    m_hardScatterChain->SetBranchAddress("ngen"      , &m_hardScatterEvent.ngen);
    m_hardScatterChain->SetBranchAddress("gen_id"    ,  m_hardScatterEvent.gen_id);
    m_hardScatterChain->SetBranchAddress("gen_status",  m_hardScatterEvent.gen_status);
    m_hardScatterChain->SetBranchAddress("gen_pt"    ,  m_hardScatterEvent.gen_pt);
    m_hardScatterChain->SetBranchAddress("gen_eta"   ,  m_hardScatterEvent.gen_eta);
    m_hardScatterChain->SetBranchAddress("gen_phi"   ,  m_hardScatterEvent.gen_phi);
    m_hardScatterChain->SetBranchAddress("gen_en"    ,  m_hardScatterEvent.gen_en);

    m_hardScatterChain->SetBranchAddress("nhits"     , &m_hardScatterEvent.nhits);
    m_hardScatterChain->SetBranchAddress("hit_type"  ,  m_hardScatterEvent.hit_type);
    m_hardScatterChain->SetBranchAddress("hit_layer" ,  m_hardScatterEvent.hit_layer);
    m_hardScatterChain->SetBranchAddress("hit_sec"   ,  m_hardScatterEvent.hit_sec);
    m_hardScatterChain->SetBranchAddress("hit_bin"   ,  m_hardScatterEvent.hit_bin);
    m_hardScatterChain->SetBranchAddress("hit_x"     ,  m_hardScatterEvent.hit_x);
    m_hardScatterChain->SetBranchAddress("hit_y"     ,  m_hardScatterEvent.hit_y);
    m_hardScatterChain->SetBranchAddress("hit_z"     ,  m_hardScatterEvent.hit_z);
    m_hardScatterChain->SetBranchAddress("hit_eta"   ,  m_hardScatterEvent.hit_eta);
    m_hardScatterChain->SetBranchAddress("hit_phi"   ,  m_hardScatterEvent.hit_phi);
    m_hardScatterChain->SetBranchAddress("hit_edep"  ,  m_hardScatterEvent.hit_edep);
    m_hardScatterChain->SetBranchAddress("hit_avgt"  ,  m_hardScatterEvent.hit_avgt);

    // Min bias tree
    m_minBiasChain->SetBranchAddress("run"       , &m_minBiasEvent.run);
    m_minBiasChain->SetBranchAddress("lumi"      , &m_minBiasEvent.lumi);
    m_minBiasChain->SetBranchAddress("event"     , &m_minBiasEvent.event);

    m_minBiasChain->SetBranchAddress("ngen"      , &m_minBiasEvent.ngen);
    m_minBiasChain->SetBranchAddress("gen_id"    ,  m_minBiasEvent.gen_id);
    m_minBiasChain->SetBranchAddress("gen_status",  m_minBiasEvent.gen_status);
    m_minBiasChain->SetBranchAddress("gen_pt"    ,  m_minBiasEvent.gen_pt);
    m_minBiasChain->SetBranchAddress("gen_eta"   ,  m_minBiasEvent.gen_eta);
    m_minBiasChain->SetBranchAddress("gen_phi"   ,  m_minBiasEvent.gen_phi);
    m_minBiasChain->SetBranchAddress("gen_en"    ,  m_minBiasEvent.gen_en);

    m_minBiasChain->SetBranchAddress("nhits"     , &m_minBiasEvent.nhits);
    m_minBiasChain->SetBranchAddress("hit_type"  ,  m_minBiasEvent.hit_type);
    m_minBiasChain->SetBranchAddress("hit_layer" ,  m_minBiasEvent.hit_layer);
    m_minBiasChain->SetBranchAddress("hit_sec"   ,  m_minBiasEvent.hit_sec);
    m_minBiasChain->SetBranchAddress("hit_bin"   ,  m_minBiasEvent.hit_bin);
    m_minBiasChain->SetBranchAddress("hit_x"     ,  m_minBiasEvent.hit_x);
    m_minBiasChain->SetBranchAddress("hit_y"     ,  m_minBiasEvent.hit_y);
    m_minBiasChain->SetBranchAddress("hit_z"     ,  m_minBiasEvent.hit_z);
    m_minBiasChain->SetBranchAddress("hit_eta"   ,  m_minBiasEvent.hit_eta);
    m_minBiasChain->SetBranchAddress("hit_phi"   ,  m_minBiasEvent.hit_phi);
    m_minBiasChain->SetBranchAddress("hit_edep"  ,  m_minBiasEvent.hit_edep);
    m_minBiasChain->SetBranchAddress("hit_avgt"  ,  m_minBiasEvent.hit_avgt);

    // Output tree
    m_outputTree->Branch("run"       , &m_mixedEvent.run       , "run/I");
    m_outputTree->Branch("lumi"      , &m_mixedEvent.lumi      , "lumi/I");
    m_outputTree->Branch("event"     , &m_mixedEvent.event     , "event/I");
    m_outputTree->Branch("nPileup"   , &m_mixedEvent.nPileup   , "nPileup/I");

    m_outputTree->Branch("ngen"      , &m_mixedEvent.ngen      , "ngen/I");
    m_outputTree->Branch("gen_id"    ,  m_mixedEvent.gen_id    , "gen_id[ngen]/I");
    m_outputTree->Branch("gen_status",  m_mixedEvent.gen_status, "gen_status[ngen]/I");
    m_outputTree->Branch("gen_pt"    ,  m_mixedEvent.gen_pt    , "gen_pt[ngen]/F");
    m_outputTree->Branch("gen_eta"   ,  m_mixedEvent.gen_eta   , "gen_eta[ngen]/F");
    m_outputTree->Branch("gen_phi"   ,  m_mixedEvent.gen_phi   , "gen_phi[ngen]/F");
    m_outputTree->Branch("gen_en"    ,  m_mixedEvent.gen_en    , "gen_en[ngen]/F");

    m_outputTree->Branch("nhits"     , &m_mixedEvent.nhits     , "nhits/I");
    m_outputTree->Branch("hit_type"  ,  m_mixedEvent.hit_type  , "hit_type[nhits]/I");
    m_outputTree->Branch("hit_layer" ,  m_mixedEvent.hit_layer , "hit_layer[nhits]/I");
    m_outputTree->Branch("hit_sec"   ,  m_mixedEvent.hit_sec   , "hit_sec[nhits]/I");
    m_outputTree->Branch("hit_bin"   ,  m_mixedEvent.hit_bin   , "hit_bin[nhits]/I");
    m_outputTree->Branch("hit_detid" ,  m_mixedEvent.hit_detid , "hit_detid[nhits]/I");
    m_outputTree->Branch("hit_x"     ,  m_mixedEvent.hit_x     , "hit_x[nhits]/F");
    m_outputTree->Branch("hit_y"     ,  m_mixedEvent.hit_y     , "hit_y[nhits]/F");
    m_outputTree->Branch("hit_z"     ,  m_mixedEvent.hit_z     , "hit_z[nhits]/F");
    m_outputTree->Branch("hit_eta"   ,  m_mixedEvent.hit_eta   , "hit_eta[nhits]/F");
    m_outputTree->Branch("hit_phi"   ,  m_mixedEvent.hit_phi   , "hit_phi[nhits]/F");
    m_outputTree->Branch("hit_edep"  ,  m_mixedEvent.hit_edep  , "hit_edep[nhits]/F");
    m_outputTree->Branch("hit_avgt"  ,  m_mixedEvent.hit_avgt  , "hit_avgt[nhits]/F");
}
