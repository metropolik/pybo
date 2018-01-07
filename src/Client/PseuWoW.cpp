#include "common.h"
#include "PseuWoW.h"
#include <time.h>
#include <openssl/rand.h>

#include "ByteBuffer.h"
//#include "DefScript/DefScript.h"
//#include "DefScriptInterface.h"
#include "Auth/BigNumber.h"
#include "Realm/RealmSession.h"
#include "World/WorldSession.h"
#include "World/CacheHandler.h"
//#include "Cli.h"
#include <stdio.h>
#include <unistd.h>



PseuInstance::PseuInstance(PseuInstanceRunnable *run)
{
    _runnable=run;
    _ver="Pybo" DEBUG_APPENDIX;
    _ver_short="A13.51" DEBUG_APPENDIX;
    _wsession=NULL;
    _rsession=NULL;
    _conf=NULL;
    _stop=false;
    _fastquit=false;
    _startrealm=true;
    _createws=false;
    _creaters=false;
    _error=false;
    _initialized=false;

}

PseuInstance::~PseuInstance()
{

    if(_rsession)
        delete _rsession;
    if(_wsession)
        delete _wsession;

    //delete _scp;
    delete _conf;


    log("--- Instance shut down ---");
}

bool PseuInstance::Init(void)
{
    log_setloglevel(0);    
    log("--- Initializing Instance ---");

    srand((unsigned)time(NULL));
    RAND_set_rand_method(RAND_SSLeay()); // init openssl randomizer

    _conf=new PseuInstanceConf();    

    CreateDir("cache");

    _conf->ApplyFromVarSet();


    if(_error)
    {
        logcritical("Errors while initializing!");
        return false;
    }

    log("Init complete.");
    _initialized=true;
    return true;
}


int PseuInstance::Run(void)
{
    if(!_initialized)
        return -1;

    logdetail("Initialized and running!");

    if(GetConf()->realmlist.empty() || GetConf()->realmport==0)
    {
        logcritical("Realmlist address not set, can't connect.");
        SetError();
    }
    else
    {        
        if(!(GetConf()->accname.empty() || GetConf()->accpass.empty()) )
        {
            logdebug("GUI not active or Login data pre-entered, skipping Login GUI");
            CreateRealmSession();
        }

        // this is the mainloop
        while(!_stop)
        {
            Update();
            if(_error)
                _stop=true;
        }
    }
    return _error;

}

void PseuInstance::Update()
{

    // delete sessions if they are no longer needed
    if(_rsession && _rsession->MustDie())
    {
        delete _rsession;
        _rsession = NULL;
    }

    if(_wsession && _wsession->MustDie())
    {
        delete _wsession;
        _wsession = NULL;
    }

    if(_createws)
    {
        _createws = false;
        if(_wsession)
            delete _wsession;
        _wsession = new WorldSession(this);
        _wsession->Start();
    }

    if(_creaters)
    {
        _creaters = false;
        if(_rsession)
            delete _rsession;
        ConnectToRealm();
    }

    // if we have no active sessions, we may reconnect, if no GUI is active for login
    if((!_rsession) && (!_wsession) && GetConf()->reconnect)
    {
        if(GetConf()->accname.empty() || GetConf()->accpass.empty())
        {
            logdev("acc name or password not set");
        }
        else
        {
            logdetail("Waiting 2sec before reconnecting.");
            sleep(2);
            CreateRealmSession();
        }
    }    

    // update currently existing/active sessions
    if(_rsession)
        _rsession->Update();
    if(_wsession)
        try { _wsession->Update(); } catch (...)
        {
            logerror("Unhandled exception in WorldSession::Update()");
        }


    usleep(GetConf()->networksleeptime * 1000); //if networksleeptime is 1 then it wait 1ms
}


void PseuInstance::SaveAllCache(void)
{
    //..
    if(GetWSession())
    {
        GetWSession()->plrNameCache.SaveToFile();
        ItemProtoCache_WriteDataToCache(GetWSession());
        CreatureTemplateCache_WriteDataToCache(GetWSession());
        GOTemplateCache_WriteDataToCache(GetWSession());
        //...
    }
}


bool PseuInstance::ConnectToRealm(void)
{
    _rsession = new RealmSession(this);
    _rsession->SetLogonData(); // get accname & accpass from PseuInstanceConfig and set it in the realm session
    _rsession->Connect();
    if(_rsession->MustDie() || !_rsession->SocketGood()) // something failed. it will be deleted in next Update() call
    {
        logerror("PseuInstance: Connecting to Realm failed!");
    }

    _rsession->SendLogonChallenge();
    return true;
}


PseuInstanceConf::PseuInstanceConf()
{
    exitonerror=false;
}

void PseuInstanceConf::ApplyFromVarSet(void)
{
    // 0 - No debug output (white text) (Default)
    // 1 - Log some details (cyan text)
    // 2 - Full debug log (dark blue text)
    // 3 - Even more debug logging (purple text, useful only for developers)
    debug=1;
    realmlist="localhost";
    accname="myacc";
    accpass="mypass2";
    exitonerror=true;
    reconnect=10000;
    realmport=3724;
    client=CLIENT_WOTLK;
    clientlang="enUS";
    realmname="Trinity";
    charname="Elodin";
    networksleeptime=1;
    // 0 - show none (Default)
    // 1 - show only known/handled
    // 2 - show only unknown/unhandled
    // 3 - show all
    showopcodes=0;
    hidefreqopcodes=true;
    hideDisabledOpcodes=true;
    enablecli=true;
    allowgamecmd=false;
    notifyping=true;
    showmyopcodes=true;
    disablespellcheck=false;
    rmcontrolport=0;
    rmcontrolhost="";
    rmcontrolpass="";
    useMaps=false;
    skipaddonchat=true;
    dumpPackets=1;
    softquit=true;
    dataLoaderThreads=2;

    switch(client)
    {
      case CLIENT_WOTLK:
      {
        clientbuild = 12340;
        clientversion_string="3.3.5";
        break;
      }
      default:
      {
        logerror("Unknown client - check conf");
      }
    }

    // clientversion is a bit more complicated to add
    std::string opt=clientversion_string + ".";
    std::string num;
    uint8 p=0;
    for(uint8 i=0;i<opt.length();i++)
    {
        if(!isdigit(opt.at(i)))
        {
            clientversion[p]=(unsigned char)atoi(num.c_str());
            num.clear();
            p++;
            if(p>2)
                break;
            continue;
        }
        num+=opt.at(i);
    }

    // cleanups, internal settings, etc.
    //log_setloglevel(1);
    log_setlogtime(false);
}




PseuInstanceConf::~PseuInstanceConf()
{
}




