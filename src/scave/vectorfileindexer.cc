//=========================================================================
//  VECTORFILEINDEXER.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <sys/stat.h>
#include <errno.h>
#include <sstream>
#include <ostream>
#include "stringutil.h"
#include "resultfilemanager.h"
#include "nodetype.h"
#include "nodetyperegistry.h"
#include "dataflowmanager.h"
#include "vectorfilereader.h"
#include "indexedvectorfile.h"
#include "indexfile.h"
#include "vectorfileindexer.h"

static inline bool existsFile(const std::string fileName)
{
    struct stat s;
    return stat(fileName.c_str(), &s)==0;
}

static std::string createTempFileName(const std::string baseFileName)
{
    std::string prefix = baseFileName;
    prefix.append(".temp");
    std::string tmpFileName = prefix;
    int serial = 0;
    char buffer[11];

    while (existsFile(tmpFileName))
        tmpFileName = opp_stringf("%s%d", prefix.c_str(), serial++);
    return tmpFileName;
}

void VectorFileIndexer::generateIndex(const char* fileName)
{
        // load file
        ResultFileManager resultFileManager;
        ResultFile *f = resultFileManager.loadFile(fileName); // TODO: limit number of lines read
        if (!f)
        {
            throw opp_runtime_error("Error: %s: load() returned null", fileName);
        }
        else if (f->numUnrecognizedLines>0)
        {
            fprintf(stderr, "WARNING: %s: %d invalid/incomplete lines out of %d\n", fileName, f->numUnrecognizedLines, f->numLines);
        }

        RunList runs = resultFileManager.getRunsInFile(f);
        if (runs.size() != 1)
        {
            if (runs.size() == 0)
                fprintf(stderr, "WARNING: %s: contains no runs\n", fileName);
            else
                fprintf(stderr, "WARNING: %s: contains %d runs, expected 1\n", fileName, (int)runs.size());
            return;
        }

        Run *runRef = runs[0];

        //
        // assemble dataflow network for vectors
        //
        DataflowManager *dataflowManager = new DataflowManager;

        // create filereader node
        NodeType *readerNodeType=NodeTypeRegistry::instance()->getNodeType("vectorfilereader");
        StringMap attrs;
        attrs["filename"] = fileName;
        VectorFileReaderNode *reader = (VectorFileReaderNode*)readerNodeType->create(dataflowManager, attrs);

        // create filewriter node
        NodeType *writerNodeType=NodeTypeRegistry::instance()->getNodeType("indexedvectorfilewriter");
        std::string tmpFileName=createTempFileName(fileName);
        attrs.clear();
        attrs["fileheader"]=generateHeader(runRef);
        attrs["blocksize"]="65536";
        attrs["filename"]=tmpFileName;
        attrs["indexfilename"]=IndexFile::getIndexFileName(fileName);
        IndexedVectorFileWriterNode *writer = (IndexedVectorFileWriterNode*)writerNodeType->create(dataflowManager, attrs);

        // create a ports for each vector on reader node and writer node and connect them
        IDList vectorIDList = resultFileManager.getAllVectors();
        for (int i=0; i<vectorIDList.size(); i++)
        {
             const VectorResult& vector = resultFileManager.getVector(vectorIDList.get(i));
             Port *readerPort = reader->addVector(vector.vectorId);
             Port *writerPort = writer->addVector(vector.vectorId, *(vector.moduleNameRef), *(vector.nameRef));
             dataflowManager->connect(readerPort, writerPort);
        }

        // run!
        dataflowManager->execute();
        delete dataflowManager;

        // rename
        if (unlink(fileName)!=0 && errno!=ENOENT)
            throw opp_runtime_error("Cannot remove original file `%s': %s", fileName, strerror(errno));
        else if (rename(tmpFileName.c_str(), fileName)!=0)
            throw opp_runtime_error("Cannot rename vector file from '%s' to '%s': %s", tmpFileName.c_str(), fileName, strerror(errno));
}

std::string VectorFileIndexer::generateHeader(Run *runRef) {
    std::stringstream header;

    header << "# generated by scavetool\n";
    header << "run " << runRef->runName << "\n";

    for (StringMap::iterator attrRef = runRef->attributes.begin(); attrRef != runRef->attributes.end(); ++attrRef)
    {
        header << "attr " << attrRef->first << " " << QUOTE(attrRef->second.c_str()) << "\n";
    }

    for (StringMap::iterator paramRef = runRef->moduleParams.begin(); paramRef != runRef->moduleParams.end(); ++paramRef)
    {
        header << "param " << paramRef->first << " " << QUOTE(paramRef->second.c_str()) << "\n";
    }

    return header.str();
}
