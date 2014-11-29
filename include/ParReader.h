/**
 *  @file  ParReader.h
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/04/2014
 *
 *  @internal
 *     Created :  06/04/2014
 * Last update :  06/04/2014 01:37:00 PM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */






#ifndef PARREADER_H
#define PARREADER_H

#include <TEnv.h>
#include <string>
#include <vector>
#include <algorithm>

namespace PileupMixing
{
    class ParReader
    {
        public:
            ParReader();
            ~ParReader(){};
            bool read(const std::string& parFileName);

            std::string& hardScatterFile(){return m_hardScatterFile;}
            std::string& minBiasFile(){return m_minBiasFile;}
            std::string& outputFile(){return m_outputFile;}
            std::string& tree(){return m_tree;}
            int firstEntry() {return m_firstEntry;}
            int lastEntry() {return m_lastEntry;}
            int nPileup(){return m_nPileup;}
            int randomSeed(){return m_randomSeed;}
            int hitEnergyThreshold(){return m_hitEnergyThreshold;}

            TEnv& params() {return m_params;}

        private:
            TEnv m_params;
            std::string m_hardScatterFile;
            std::string m_minBiasFile;
            std::string m_outputFile;
            std::string m_tree;
            int m_firstEntry;
            int m_lastEntry;
            int m_nPileup;
            int m_randomSeed;
            float m_hitEnergyThreshold;
    };

}



#endif
