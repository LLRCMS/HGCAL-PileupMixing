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



class TFile;
class TChain;
class TTree;
class TEntryList;

namespace PileupMixing
{

    struct HGCSimEvent 
    { 
        int   event, lumi, run, npu;
        int   gen_n;
        std::vector<int>   *gen_id    ;
        std::vector<int>   *gen_status;
        std::vector<float> *gen_eta   ;
        std::vector<float> *gen_phi   ;
        std::vector<float> *gen_pt   ;
        std::vector<float> *gen_energy;
        int   hit_n;
        std::vector<uint32_t> *hit_detid ;
        std::vector<int>      *hit_subdet;
        std::vector<int>      *hit_cell  ;
        std::vector<int>      *hit_sector;
        std::vector<int>      *hit_subsector;
        std::vector<int>      *hit_layer ;
        std::vector<int>      *hit_zside;
        std::vector<float>    *hit_energy;
        std::vector<float>    *hit_eta   ;
        std::vector<float>    *hit_phi   ;
        std::vector<float>    *hit_x     ;
        std::vector<float>    *hit_y     ;
        std::vector<float>    *hit_z     ;

    } ;

    struct HGCSimGen
    {
        int   id ;
        int   status ;
        float eta  ;
        float phi  ;
        float pt  ;
        float energy;
    } ;

    struct HGCSimHit
    {
        uint32_t detid ;
        int      subdet;
        int      cell  ;
        int      sector;
        int      subsector;
        int      layer ;
        int      zside;
        float    energy;
        float    eta   ;
        float    phi   ;
        float    x     ;
        float    y     ;
        float    z     ;

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
            std::map<uint32_t, HGCSimHit> m_hits;
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
