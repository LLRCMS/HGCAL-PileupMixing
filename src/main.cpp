
#include <string>
#include <iostream>

#include "Mixer.h"

using namespace std;
using namespace PileupMixing;

int main(int argc, char** argv)
{


    if(argc!=2)
    {
        cout<<"Usage: mixing.exe parFile\n";
        return 1;
    }
    string parFile(argv[1]);

    Mixer* mixer = new Mixer();
    
    try
    {
        bool status = mixer->initialize(parFile);
        if(!status)
        {
            delete mixer;
            return 1;
        }
        mixer->loop();
    }
    catch(string s)
    {
        delete mixer;
        return 1;
    }

    delete mixer;
    return 0;
}
