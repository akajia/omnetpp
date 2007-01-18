//=========================================================================
//  VECTORFILEINDEXER.H - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef _IVECTORFILEINDEXER_H_
#define _IVECTORFILEINDEXER_H_

#include <string>
#include "resultfilemanager.h"

/**
 * FIXME comment
 */
class VectorFileIndexer
{
    public:
        void generateIndex(const char *filename);
    protected:
        std::string generateHeader(Run *runRef);
};

#endif

