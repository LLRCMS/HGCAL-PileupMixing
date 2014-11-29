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
#include "TSystem.h"

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
    gSystem->Load("obj/libDictionary_C.so");

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
    m_mixedEvent.npu = nPileup;
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
        overlapHits(m_minBiasEvent);
    }
}


/*****************************************************************/
void Mixer::overlapHits(HGCSimEvent& event)
/*****************************************************************/
{
    int nHits = event.hit_n;
    for(int h=0; h<nHits; h++)
    {
        uint32_t detid = event.hit_detid->at(h);
        //int z = (event.hit_zside->at(h)==-1 ? 0 : 1);
        //int l = event.hit_layer->at(h)-1;
        //int s = event.hit_sector->at(h)-1;
        //int ss = (event.hit_subsector->at(h)==-1 ? 0 : 1);
        //int c = event.hit_cell->at(h);
        //unsigned detid = (unsigned)(c + ss*2400 + s*4800 + l*86400 + z*2592000);
        auto itrHit = m_hits.find(detid);
        if(itrHit==m_hits.end())
        {
            // create new hit
            HGCSimHit hit;
            hit.detid     = event.hit_detid->at(h);//detid;
            hit.cell      = event.hit_cell->at(h);
            hit.subdet    = event.hit_subdet->at(h);
            hit.sector    = event.hit_sector->at(h);
            hit.subsector = event.hit_subsector->at(h);
            hit.layer     = event.hit_layer->at(h);
            hit.zside     = event.hit_zside->at(h);
            hit.energy    = event.hit_energy->at(h);
            hit.eta       = event.hit_eta->at(h);
            hit.phi       = event.hit_phi->at(h);
            hit.x         = event.hit_x->at(h);
            hit.y         = event.hit_y->at(h);
            hit.z         = event.hit_z->at(h);

            m_hits[detid] = hit;
        }
        else
        {
            // sum hit energy. 
            HGCSimHit& hit = itrHit->second;
            hit.energy += event.hit_energy->at(h);
        }
    }
}

/*****************************************************************/
void Mixer::overlapGen(HGCSimEvent& event)
/*****************************************************************/
{
    int nGen = event.gen_n;
    for(int g=0; g<nGen; g++)
    {
        // create new gen
        HGCSimGen gen;
        gen.id     = event.gen_id->at(g);
        gen.status = event.gen_status->at(g);
        gen.eta    = event.gen_eta->at(g);
        gen.phi    = event.gen_phi->at(g);
        gen.pt     = event.gen_pt->at(g);
        gen.energy = event.gen_energy->at(g);
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
    m_mixedEvent.hit_n = 0;
    m_mixedEvent.gen_n = 0;

    const double mip = 0.000055;

    for(auto itrHit=m_hits.begin(); itrHit!=m_hits.end(); ++itrHit)
    {
        const HGCSimHit& hit = itrHit->second;
        if(hit.energy<=m_reader.hitEnergyThreshold()*mip) continue;

        m_mixedEvent.hit_n++;
        m_mixedEvent.hit_detid    ->push_back(hit.detid);
        m_mixedEvent.hit_cell     ->push_back(hit.cell);
        m_mixedEvent.hit_subdet   ->push_back(hit.subdet);
        m_mixedEvent.hit_sector   ->push_back(hit.sector);
        m_mixedEvent.hit_subsector->push_back(hit.subsector);
        m_mixedEvent.hit_layer    ->push_back(hit.layer);
        m_mixedEvent.hit_zside    ->push_back(hit.zside);
        m_mixedEvent.hit_energy   ->push_back(hit.energy);
        m_mixedEvent.hit_eta      ->push_back(hit.eta);
        m_mixedEvent.hit_phi      ->push_back(hit.phi);
        m_mixedEvent.hit_x        ->push_back(hit.x);
        m_mixedEvent.hit_y        ->push_back(hit.y);
        m_mixedEvent.hit_z        ->push_back(hit.z);

    }
    m_mixedEvent.hit_detid    ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_cell     ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_subdet   ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_sector   ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_subsector->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_layer    ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_zside    ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_energy   ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_eta      ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_phi      ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_x        ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_y        ->resize(m_mixedEvent.hit_n);
    m_mixedEvent.hit_z        ->resize(m_mixedEvent.hit_n);


    for(auto itrGen=m_gens.begin(); itrGen!=m_gens.end(); ++itrGen)
    {
        const HGCSimGen& gen = *itrGen;
        m_mixedEvent.gen_n++;
        m_mixedEvent.gen_id    ->push_back(gen.id);
        m_mixedEvent.gen_status->push_back(gen.status);
        m_mixedEvent.gen_eta   ->push_back(gen.eta);
        m_mixedEvent.gen_phi   ->push_back(gen.phi);
        m_mixedEvent.gen_pt    ->push_back(gen.pt);
        m_mixedEvent.gen_energy->push_back(gen.energy);
    }
    m_mixedEvent.gen_id    ->resize(m_mixedEvent.gen_n);
    m_mixedEvent.gen_status->resize(m_mixedEvent.gen_n);
    m_mixedEvent.gen_eta   ->resize(m_mixedEvent.gen_n);
    m_mixedEvent.gen_phi   ->resize(m_mixedEvent.gen_n);
    m_mixedEvent.gen_pt    ->resize(m_mixedEvent.gen_n);
    m_mixedEvent.gen_energy->resize(m_mixedEvent.gen_n);

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
    m_mixedEvent.gen_n = 0;
    m_mixedEvent.hit_n = 0;
    m_mixedEvent.gen_id    ->clear();
    m_mixedEvent.gen_status->clear();
    m_mixedEvent.gen_eta   ->clear();
    m_mixedEvent.gen_phi   ->clear();
    m_mixedEvent.gen_pt    ->clear();
    m_mixedEvent.gen_energy->clear();

    m_mixedEvent.hit_detid    ->clear();
    m_mixedEvent.hit_subdet   ->clear();
    m_mixedEvent.hit_cell     ->clear();
    m_mixedEvent.hit_sector   ->clear();
    m_mixedEvent.hit_subsector->clear();
    m_mixedEvent.hit_layer    ->clear();
    m_mixedEvent.hit_zside    ->clear();
    m_mixedEvent.hit_energy   ->clear();
    m_mixedEvent.hit_x        ->clear();
    m_mixedEvent.hit_y        ->clear();
    m_mixedEvent.hit_z        ->clear();
    m_mixedEvent.hit_eta      ->clear();
    m_mixedEvent.hit_phi      ->clear();
}


/*****************************************************************/
void Mixer::branch()
/*****************************************************************/
{
    m_mixedEvent.gen_id        = 0;
    m_mixedEvent.gen_status    = 0;
    m_mixedEvent.gen_eta       = 0;
    m_mixedEvent.gen_phi       = 0;
    m_mixedEvent.gen_pt        = 0;
    m_mixedEvent.gen_energy    = 0;
    //
    m_mixedEvent.hit_detid     = 0;
    m_mixedEvent.hit_subdet    = 0;
    m_mixedEvent.hit_cell      = 0;
    m_mixedEvent.hit_sector    = 0;
    m_mixedEvent.hit_subsector = 0;
    m_mixedEvent.hit_layer     = 0;
    m_mixedEvent.hit_zside     = 0;
    m_mixedEvent.hit_energy    = 0;
    m_mixedEvent.hit_x         = 0;
    m_mixedEvent.hit_y         = 0;
    m_mixedEvent.hit_z         = 0;
    m_mixedEvent.hit_eta       = 0;
    m_mixedEvent.hit_phi       = 0;
    //
    m_hardScatterEvent.hit_detid     = 0;
    m_hardScatterEvent.hit_subdet    = 0;
    m_hardScatterEvent.hit_cell      = 0;
    m_hardScatterEvent.hit_sector    = 0;
    m_hardScatterEvent.hit_subsector = 0;
    m_hardScatterEvent.hit_layer     = 0;
    m_hardScatterEvent.hit_zside     = 0;
    m_hardScatterEvent.hit_energy    = 0;
    m_hardScatterEvent.hit_x         = 0;
    m_hardScatterEvent.hit_y         = 0;
    m_hardScatterEvent.hit_z         = 0;
    m_hardScatterEvent.hit_eta       = 0;
    m_hardScatterEvent.hit_phi       = 0;


    // Hard scatter tree
    m_hardScatterChain->SetBranchAddress("run"       , &m_hardScatterEvent.run);
    m_hardScatterChain->SetBranchAddress("lumi"      , &m_hardScatterEvent.lumi);
    m_hardScatterChain->SetBranchAddress("event"     , &m_hardScatterEvent.event);

    m_hardScatterChain->SetBranchAddress("gen_n"     , &m_hardScatterEvent.gen_n);
    m_hardScatterChain->SetBranchAddress("gen_id"    , &m_hardScatterEvent.gen_id);
    m_hardScatterChain->SetBranchAddress("gen_status", &m_hardScatterEvent.gen_status);
    m_hardScatterChain->SetBranchAddress("gen_eta"   , &m_hardScatterEvent.gen_eta);
    m_hardScatterChain->SetBranchAddress("gen_phi"   , &m_hardScatterEvent.gen_phi);
    m_hardScatterChain->SetBranchAddress("gen_pt"    , &m_hardScatterEvent.gen_pt);
    m_hardScatterChain->SetBranchAddress("gen_energy", &m_hardScatterEvent.gen_energy);

    m_hardScatterChain->SetBranchAddress("hit_n"        , &m_hardScatterEvent.hit_n);
    m_hardScatterChain->SetBranchAddress("hit_detid"    , &m_hardScatterEvent.hit_detid);
    m_hardScatterChain->SetBranchAddress("hit_cell"     , &m_hardScatterEvent.hit_cell);
    m_hardScatterChain->SetBranchAddress("hit_subdet"   , &m_hardScatterEvent.hit_subdet);
    m_hardScatterChain->SetBranchAddress("hit_layer"    , &m_hardScatterEvent.hit_layer);
    m_hardScatterChain->SetBranchAddress("hit_zside"    , &m_hardScatterEvent.hit_zside);
    m_hardScatterChain->SetBranchAddress("hit_sector"   , &m_hardScatterEvent.hit_sector);
    m_hardScatterChain->SetBranchAddress("hit_subsector", &m_hardScatterEvent.hit_subsector);
    m_hardScatterChain->SetBranchAddress("hit_energy"   , &m_hardScatterEvent.hit_energy);
    m_hardScatterChain->SetBranchAddress("hit_x"        , &m_hardScatterEvent.hit_x);
    m_hardScatterChain->SetBranchAddress("hit_y"        , &m_hardScatterEvent.hit_y);
    m_hardScatterChain->SetBranchAddress("hit_z"        , &m_hardScatterEvent.hit_z);
    m_hardScatterChain->SetBranchAddress("hit_eta"      , &m_hardScatterEvent.hit_eta);
    m_hardScatterChain->SetBranchAddress("hit_phi"      , &m_hardScatterEvent.hit_phi);

    // Min bias tree
    m_minBiasChain->SetBranchAddress("run"       , &m_minBiasEvent.run);
    m_minBiasChain->SetBranchAddress("lumi"      , &m_minBiasEvent.lumi);
    m_minBiasChain->SetBranchAddress("event"     , &m_minBiasEvent.event);

    m_minBiasChain->SetBranchAddress("gen_n"     , &m_minBiasEvent.gen_n);
    m_minBiasChain->SetBranchAddress("gen_id"    , &m_minBiasEvent.gen_id);
    m_minBiasChain->SetBranchAddress("gen_status", &m_minBiasEvent.gen_status);
    m_minBiasChain->SetBranchAddress("gen_eta"   , &m_minBiasEvent.gen_eta);
    m_minBiasChain->SetBranchAddress("gen_phi"   , &m_minBiasEvent.gen_phi);
    m_minBiasChain->SetBranchAddress("gen_pt"    , &m_minBiasEvent.gen_pt);
    m_minBiasChain->SetBranchAddress("gen_energy", &m_minBiasEvent.gen_energy);

    m_minBiasChain->SetBranchAddress("hit_n"        , &m_minBiasEvent.hit_n);
    m_minBiasChain->SetBranchAddress("hit_detid"    , &m_minBiasEvent.hit_detid);
    m_minBiasChain->SetBranchAddress("hit_cell"     , &m_minBiasEvent.hit_cell);
    m_minBiasChain->SetBranchAddress("hit_subdet"   , &m_minBiasEvent.hit_subdet);
    m_minBiasChain->SetBranchAddress("hit_layer"    , &m_minBiasEvent.hit_layer);
    m_minBiasChain->SetBranchAddress("hit_zside"    , &m_minBiasEvent.hit_zside);
    m_minBiasChain->SetBranchAddress("hit_sector"   , &m_minBiasEvent.hit_sector);
    m_minBiasChain->SetBranchAddress("hit_subsector", &m_minBiasEvent.hit_subsector);
    m_minBiasChain->SetBranchAddress("hit_energy"   , &m_minBiasEvent.hit_energy);
    m_minBiasChain->SetBranchAddress("hit_x"        , &m_minBiasEvent.hit_x);
    m_minBiasChain->SetBranchAddress("hit_y"        , &m_minBiasEvent.hit_y);
    m_minBiasChain->SetBranchAddress("hit_z"        , &m_minBiasEvent.hit_z);
    m_minBiasChain->SetBranchAddress("hit_eta"      , &m_minBiasEvent.hit_eta);
    m_minBiasChain->SetBranchAddress("hit_phi"      , &m_minBiasEvent.hit_phi);

    // Output tree
    m_outputTree->Branch("run"       , &m_mixedEvent.run       , "run/I");
    m_outputTree->Branch("lumi"      , &m_mixedEvent.lumi      , "lumi/I");
    m_outputTree->Branch("event"     , &m_mixedEvent.event     , "event/I");
    m_outputTree->Branch("npu"       , &m_mixedEvent.npu       , "npu/I");

    m_outputTree->Branch("gen_n"      , &m_mixedEvent.gen_n      , "gen_n/I");
    m_outputTree->Branch("gen_id"    ,  &m_mixedEvent.gen_id    );
    m_outputTree->Branch("gen_status",  &m_mixedEvent.gen_status);
    m_outputTree->Branch("gen_eta"   ,  &m_mixedEvent.gen_eta   );
    m_outputTree->Branch("gen_phi"   ,  &m_mixedEvent.gen_phi   );
    m_outputTree->Branch("gen_pt"    ,  &m_mixedEvent.gen_pt    );
    m_outputTree->Branch("gen_energy",  &m_mixedEvent.gen_energy);

    m_outputTree->Branch("hit_n"        , &m_mixedEvent.hit_n     , "hit_n/I");
    m_outputTree->Branch("hit_detid"    , &m_mixedEvent.hit_detid);
    m_outputTree->Branch("hit_subdet"   , &m_mixedEvent.hit_subdet);
    m_outputTree->Branch("hit_cell"     , &m_mixedEvent.hit_cell   );
    m_outputTree->Branch("hit_layer"    , &m_mixedEvent.hit_layer );
    m_outputTree->Branch("hit_zside"    , &m_mixedEvent.hit_zside );
    m_outputTree->Branch("hit_sector"   , &m_mixedEvent.hit_sector);
    m_outputTree->Branch("hit_subsector", &m_mixedEvent.hit_subsector);
    m_outputTree->Branch("hit_energy"   , &m_mixedEvent.hit_energy);
    m_outputTree->Branch("hit_x"        , &m_mixedEvent.hit_x     );
    m_outputTree->Branch("hit_y"        , &m_mixedEvent.hit_y     );
    m_outputTree->Branch("hit_z"        , &m_mixedEvent.hit_z     );
    m_outputTree->Branch("hit_eta"      , &m_mixedEvent.hit_eta   );
    m_outputTree->Branch("hit_phi"      , &m_mixedEvent.hit_phi   );

    m_outputTree->Branch("hard_hit_n"        , &m_hardScatterEvent.hit_n     , "hard_hit_n/I");
    m_outputTree->Branch("hard_hit_detid"    , &m_hardScatterEvent.hit_detid  );
    m_outputTree->Branch("hard_hit_subdet"   , &m_hardScatterEvent.hit_subdet  );
    m_outputTree->Branch("hard_hit_cell"     , &m_hardScatterEvent.hit_cell   );
    m_outputTree->Branch("hard_hit_layer"    , &m_hardScatterEvent.hit_layer );
    m_outputTree->Branch("hard_hit_zside"    , &m_hardScatterEvent.hit_zside );
    m_outputTree->Branch("hard_hit_sector"   , &m_hardScatterEvent.hit_sector   );
    m_outputTree->Branch("hard_hit_subsector", &m_hardScatterEvent.hit_subsector   );
    m_outputTree->Branch("hard_hit_energy"   , &m_hardScatterEvent.hit_energy);
    m_outputTree->Branch("hard_hit_x"        , &m_hardScatterEvent.hit_x     );
    m_outputTree->Branch("hard_hit_y"        , &m_hardScatterEvent.hit_y     );
    m_outputTree->Branch("hard_hit_z"        , &m_hardScatterEvent.hit_z     );
    m_outputTree->Branch("hard_hit_eta"      , &m_hardScatterEvent.hit_eta   );
    m_outputTree->Branch("hard_hit_phi"      , &m_hardScatterEvent.hit_phi   );
}
