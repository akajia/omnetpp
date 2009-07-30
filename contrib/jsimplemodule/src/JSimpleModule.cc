#include <assert.h>
#include <string.h>
#include <algorithm>
#include "JSimpleModule.h"
#include "JUtil.h"

using namespace JUtil;  // for jenv, checkExceptions(), findMethod(), etc


//#define DEBUGPRINTF printf
#define DEBUGPRINTF (void)

Define_Module(JSimpleModule);


JSimpleModule::JSimpleModule()
{
    javaPeer = 0;
}

JSimpleModule::~JSimpleModule()
{
    if (javaPeer && finishMethod)
        jenv->DeleteGlobalRef(javaPeer);
}

int JSimpleModule::numInitStages() const
{
    if (javaPeer==0)
        return 1; // at the beginning, we can only say "at least one stage"

    int n = jenv->CallIntMethod(javaPeer, numInitStagesMethod);
    checkExceptions();
    return n;
}

void JSimpleModule::initialize(int stage)
{
    if (stage==0)
        createJavaModuleObject();

    DEBUGPRINTF("Invoking initialize(%d) on new instance...\n", stage);
    jenv->CallVoidMethod(javaPeer, initializeStageMethod, stage);
    checkExceptions();
}

void JSimpleModule::createJavaModuleObject()
{
    // create VM if needed
    if (!jenv)
         initJVM();

    // find class and method IDs (note: initialize() and finish() are optional)
    std::string className = getNedTypeName();
    replace(className.begin(), className.end(), '.', '/');
    const char *clazzName = className.c_str();
    DEBUGPRINTF("Finding class %s...\n", clazzName);
    jclass clazz = jenv->FindClass(clazzName);
    checkExceptions();

    DEBUGPRINTF("Finding initialize()/handleMessage()/finish() methods for class %s...\n", clazzName);
    numInitStagesMethod = findMethod(clazz, clazzName, "numInitStages", "()I");
    initializeStageMethod = findMethod(clazz, clazzName, "initialize", "(I)V");
    doHandleMessageMethod = findMethod(clazz, clazzName, "doHandleMessage", "()V");
    finishMethod = findMethod(clazz, clazzName, "finish", "()V");

    // create Java module object
    DEBUGPRINTF("Instantiating class %s...\n", clazzName);

    jmethodID setCPtrMethod = jenv->GetMethodID(clazz, "setCPtr", "(J)V");
    jenv->ExceptionClear();
    if (setCPtrMethod)
    {
        jmethodID ctor = findMethod(clazz, clazzName, "<init>", "()V");
        javaPeer = jenv->NewObject(clazz, ctor);
        checkExceptions();
        jenv->CallVoidMethod(javaPeer, setCPtrMethod, (jlong)this);
    }
    else
    {
        jmethodID ctor = findMethod(clazz, clazzName, "<init>", "(J)V");
        javaPeer = jenv->NewObject(clazz, ctor, (jlong)this);
        checkExceptions();
    }
    javaPeer = jenv->NewGlobalRef(javaPeer);
    checkExceptions();
    JObjectAccess::setObject(javaPeer);
}

void JSimpleModule::handleMessage(cMessage *msg)
{
    msgToBeHandled = msg;
    jenv->CallVoidMethod(javaPeer, doHandleMessageMethod);
    checkExceptions();
}

void JSimpleModule::finish()
{
    jenv->CallVoidMethod(javaPeer, finishMethod);
    checkExceptions();
}

void JSimpleModule::swigSetJavaPeer(jobject moduleObject)
{
    ASSERT(javaPeer==0);
    javaPeer = jenv->NewGlobalRef(moduleObject);
    JObjectAccess::setObject(javaPeer);
}

jobject JSimpleModule::swigJavaPeerOf(cSimpleModule *object)
{
    JSimpleModule *mod = dynamic_cast<JSimpleModule *>(object);
    return mod ? mod->swigJavaPeer() : 0;
}

