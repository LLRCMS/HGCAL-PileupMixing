/**
 *  @file  ParReader.cpp
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/04/2014
 *
 *  @internal
 *     Created :  06/04/2014
 * Last update :  06/04/2014 01:43:06 PM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */





#include "ParReader.h"
#include <iostream>

using namespace PileupMixing;
using namespace std;


/*****************************************************************/
ParReader::ParReader():m_hardScatterFile(""),
    m_minBiasFile(""),
    m_outputFile("dummy.root"),
    m_tree("dummyTree"),
    m_firstEntry(0),
    m_lastEntry(-1),
    m_nPileup(200),
    m_randomSeed(123456)
/*****************************************************************/
{
}


/*****************************************************************/
bool ParReader::read(const std::string& parFileName)
/*****************************************************************/
{
    int status = m_params.ReadFile(parFileName.c_str(),EEnvLevel(0));
    if(status!=0) 
    {
        cout<<"FATAL: ParReader::read(): Cannot read file "<<parFileName<<"\n"; 
        return false;
    }

    m_hardScatterFile = m_params.GetValue("HardScatterFile", "");
    m_minBiasFile     = m_params.GetValue("MinBiasFile", "");
    m_outputFile      = m_params.GetValue("OutputFile", "dummy.root");
    m_tree            = m_params.GetValue("Tree", "dummyTree");
    m_firstEntry      = m_params.GetValue("FirstEntry", 0);
    m_lastEntry       = m_params.GetValue("LastEntry", -1);
    m_nPileup         = m_params.GetValue("NPileup", 200);
    m_randomSeed      = m_params.GetValue("RandomSeed", 123456);

    return true;
}
