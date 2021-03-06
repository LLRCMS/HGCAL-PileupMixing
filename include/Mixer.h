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
        //
        int   gen_n;
        std::vector<int>   *gen_id    ;
        std::vector<int>   *gen_status;
        std::vector<float> *gen_eta   ;
        std::vector<float> *gen_phi   ;
        std::vector<float> *gen_pt   ;
        std::vector<float> *gen_energy;
        //
        int genjet_n;
        std::vector<float> *genjet_energy;
        std::vector<float> *genjet_emenergy;
        std::vector<float> *genjet_hadenergy;
        std::vector<float> *genjet_invenergy;
        std::vector<float> *genjet_pt;
        std::vector<float> *genjet_eta;
        std::vector<float> *genjet_phi;
        //
        int gentau_n;
        std::vector<float> *gentau_energy;
        std::vector<float> *gentau_pt;
        std::vector<float> *gentau_eta;
        std::vector<float> *gentau_phi;
        std::vector<int>   *gentau_decay;
        //
        int   hit_n;
        std::vector<uint32_t> *hit_detid ;
        std::vector<uint8_t>  *hit_subdet;
        std::vector<uint16_t> *hit_cell  ;
        std::vector<uint8_t>  *hit_sector;
        std::vector<char>     *hit_subsector;
        std::vector<uint8_t>  *hit_layer ;
        std::vector<char>     *hit_zside;
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

    struct HGCSimGenJet
    {
        float energy;
        float emenergy;
        float hadenergy;
        float invenergy;
        float pt;
        float eta;
        float phi;
    } ;

    struct HGCSimGenTau
    {
        float energy;
        float pt;
        float eta;
        float phi;
        int   decay;
    } ;

    struct HGCSimHit
    {
        uint32_t detid ;
        uint8_t  subdet;
        uint16_t cell  ;
        uint8_t  sector;
        char     subsector;
        uint8_t  layer ;
        char     zside;
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
            std::vector<HGCSimGenJet> m_genjets;
            std::vector<HGCSimGenTau> m_gentaus;

            // tree variables
            HGCSimEvent m_hardScatterEvent;
            HGCSimEvent m_minBiasEvent;
            HGCSimEvent m_mixedEvent;

            // Output tree
            TTree* m_outputTree;

    };

}

#endif 
