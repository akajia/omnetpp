//==========================================================================
//  CMERSENNETWISTER.CC - part of
//                 OMNeT++/OMNEST
//              Discrete System Simulation in C++
//
// Contents:
//   class cMersenneTwister
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "cenvir.h"
#include "util.h"
#include "cexception.h"
#include "cmersennetwister.h"
#include "cmessage.h"


Register_Class(cMersenneTwister);

//XXX "seed-X-mt", etc needs to be registered, otherwise it will be an error!

void cMersenneTwister::initialize(int runNumber, int rngId, int numRngs,
                                  int parsimProcId, int parsimNumPartitions,
                                  cConfiguration *cfg)
{
    //XXX runNumber is unused (??)
    char key[40], key2[40];
    sprintf(key, "seed-%d-mt", rngId);
    sprintf(key2, "seed-%d-mt-p%d", rngId, parsimProcId);

    unsigned long seed;
    if (parsimNumPartitions>1)
    {
        // with parallel simulation, every partition should get distinct streams
        const char *value = cfg->getConfigValue(key2);
        if (value!=NULL)
        {
            seed = cConfiguration::parseLong(value, 0);
        }
        else
        {
            if (cfg->getConfigValue(key)!=NULL)
                ev << "Warning: cMersenneTwister: ignoring config key " << key << "=<seed>"
                   << " for parallel simulation -- please use partition-specific variant "
                   << key2 << "=<seed>\n";
            seed = (runNumber*numRngs + rngId)*MAX_PARSIM_PARTITIONS + parsimProcId;
        }
    }
    else
    {
        const char *value = cfg->getConfigValue(key);
        if (value!=NULL)
            seed = cConfiguration::parseLong(value, 0);
        else
            seed = runNumber*numRngs + rngId;
    }

    // use the following number as seed, and hope that all seed values
    // are well apart in the 2^19937-long sequence (it should hold if
    // someone did the work of testing the initialization routine).
    rng.seed(seed);
}

void cMersenneTwister::selfTest()
{
    rng.seed(1);
    for (int i=0; i<10000; i++)
        intRand();
    if (intRand()!=2915232614UL)
        throw cRuntimeError("cMersenneTwister: selfTest() failed, please report this problem!");
}

unsigned long cMersenneTwister::intRand()
{
    numDrawn++;
    return rng.randInt();
}

unsigned long cMersenneTwister::intRandMax()
{
    return 0xffffffffUL; // 2^32-1
}

unsigned long cMersenneTwister::intRand(unsigned long n)
{
    numDrawn++;
    return rng.randInt(n-1);
}

double cMersenneTwister::doubleRand()
{
    numDrawn++;
    return rng.randExc();
}

double cMersenneTwister::doubleRandNonz()
{
    numDrawn++;
    return rng.randDblExc();
}

double cMersenneTwister::doubleRandIncl1()
{
    numDrawn++;
    return rng.rand();
}


