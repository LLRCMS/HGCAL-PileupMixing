/**
 *  @file  Mixer.h
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/04/2014
 *
 *  @internal
 *     Created :  06/04/2014
 * Last update :  06/04/2014 01:51:07 PM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */



#ifndef MIXER_H
#define MIXER_H


#include <string>
#include <set>
#include <map>
#include <vector>

#include <TRandom3.h>

#include "ParReader.h"


//#define MAXHGCHITSPEREVENT 1000000

class TFile;
class TChain;
class TTree;
class TEntryList;

namespace PileupMixing
{
    static const int MAXGEN=10000;
    static const int MAXHIT=10000000;

    struct HGCSimEvent 
    { 
        int   event, lumi, run, nPileup;
        int   ngen;
        int   gen_id    [MAXGEN];
        int   gen_status[MAXGEN];
        float gen_pt    [MAXGEN];
        float gen_eta   [MAXGEN];
        float gen_phi   [MAXGEN];
        float gen_en    [MAXGEN];
        int   nhits;
        int   hit_type  [MAXHIT];
        int   hit_layer [MAXHIT];
        int   hit_sec   [MAXHIT];
        int   hit_bin   [MAXHIT];
        int   hit_detid [MAXHIT];
        float hit_edep  [MAXHIT];
        float hit_avgt  [MAXHIT];
        float hit_x     [MAXHIT];
        float hit_y     [MAXHIT];
        float hit_z     [MAXHIT];
        float hit_eta   [MAXHIT];
        float hit_phi   [MAXHIT];
    } ;

    struct HGCSimGen
    {
        int   id ;
        int   status ;
        float pt   ;
        float eta  ;
        float phi  ;
        float en   ;
    } ;

    struct HGCSimHit
    {
        int   type  ;
        int   layer ;
        int   sec   ;
        int   bin   ;
        int   detid ;
        float edep  ;
        float avgt  ;
        float x     ;
        float y     ;
        float z     ;
        float eta   ;
        float phi   ;
    } ;

    class Mixer
    {
        public:
            Mixer();
            ~Mixer();

            bool initialize(const std::string& parameterFile);
            void loop();

        private:
            void execute();
            void branch();
            void mix();
            void overlapHits(HGCSimEvent& event);
            void overlapGen(HGCSimEvent& event);
            void fill();
            void clean();

            std::string m_outputFileName;
            TChain* m_hardScatterChain;
            TChain* m_minBiasChain;
            Long64_t m_nEntriesHardScatter;
            Long64_t m_nEntriesMinBias;
            TFile* m_outputFile;
            ParReader m_reader;
            TRandom3 m_random;
            std::set<Long64_t> m_mixedEvents;
            std::map<int, HGCSimHit> m_hits;
            std::vector<HGCSimGen> m_gens;

            // tree variables
            HGCSimEvent m_hardScatterEvent;
            HGCSimEvent m_minBiasEvent;
            HGCSimEvent m_mixedEvent;

            // Output tree
            TTree* m_outputTree;

    };

}

#endif 
