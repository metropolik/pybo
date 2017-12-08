#include "common.h"
#include "PseuWoW.h"
#include <time.h>
#include <openssl/rand.h>

#include "ByteBuffer.h"
//#include "DefScript/DefScript.h"
//#include "DefScriptInterface.h"
#include "Auth/BigNumber.h"
#include "DefScript/DefScript.h"
#include "Realm/RealmSession.h"
#include "World/WorldSession.h"
#include "World/CacheHandler.h"
#include "RemoteController.h"
//#include "Cli.h"
#include <stdio.h>
#include <unistd.h>


//###### Start of program code #######


// void PseuInstanceRunnable::run(void)
// {
//     _i->SetConfDir("./conf/");
//     _i->SetScpDir("./scripts/");
//     if(_i->Init())
//     {
//         _i->Run();
//     }
//     else
//     {
//         getchar(); // if init failed, wait for keypress before exit
//     }
//     delete _i;
// }


PseuInstance::PseuInstance(PseuInstanceRunnable *run)
{
    _runnable=run;
    _ver="PseuWoW Alpha Build 13.51" DEBUG_APPENDIX;
    _ver_short="A13.51" DEBUG_APPENDIX;
    _wsession=NULL;
    _rsession=NULL;
    _scp=NULL;
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
    log("");
    log("--- Initializing Instance ---");

    if(_confdir.empty())
        _confdir="./conf/";
    if(_scpdir.empty())
        _scpdir="./scripts/";

    srand((unsigned)time(NULL));
    RAND_set_rand_method(RAND_SSLeay()); // init openssl randomizer

    // _scp=new DefScriptPackage();
    // _scp->SetParentMethod((void*)this);
    _conf=new PseuInstanceConf();

    // _scp->SetPath(_scpdir);

    CreateDir("cache");

    dbmgr.AddSearchPath("./cache");
    dbmgr.AddSearchPath("./data/scp");
    dbmgr.SetCompression(6);

    // _scp->variables.Set("@version_short",_ver_short);
    // _scp->variables.Set("@version",_ver);
    // _scp->variables.Set("@inworld","false");

    // if(!_scp->LoadScriptFromFile("./_startup.def"))
    // {
    //     logerror("Error loading '_startup.def'");
    //     SetError();
    // }
    // else if(!_scp->BoolRunScript("_startup",NULL))
    // {
    //     logerror("Error executing '_startup.def'");
    //     SetError();
    // }
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


void PseuInstance::Run(void)
{
    if(!_initialized)
        return;

    logdetail("PseuInstance: Initialized and running!");

    // TODO: as soon as username and password can be inputted into the gui, wait until it was set by user.
    //pybot--hier realmlist einstellen
    if(GetConf()->realmlist.empty() || GetConf()->realmport==0)
    {
        logcritical("Realmlist address not set, can't connect.");
        SetError();
    }
    else
    {
        //pybot--acc und accpass einstellen
        if(!GetConf()->enablegui || !(GetConf()->accname.empty() || GetConf()->accpass.empty()) )
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

    // fastquit is defined if we clicked [X] (on windows)
    // If softquit is set, do not terminate forcefully, but shut it down instead
    if(_fastquit && !_conf->softquit)
    {
        log("Aborting Instance...");
        return;
    }

    log("Shutting down instance...");



    // if(GetScripts()->ScriptExists("_onexit"))
    // {
    //     CmdSet Set;
    //     Set.arg[0] = DefScriptTools::toString(_error);
    //     GetScripts()->RunScript("_onexit",&Set);
    // }

    if(GetConf()->exitonerror == false && _error)
    {
        log("Exiting on error is disabled, PseuWoW is now IDLE");
        log("-- Press enter to exit --");
        char crap[100];
        fgets(crap,sizeof(crap),stdin); // workaround, need to press enter 2x for now
    }

}

void PseuInstance::Update()
{
    // if the user typed anything into the console, process it before anything else.
    // note that it can also be used for simulated cli commands sent by other threads, so it needs to be checked even if cli is disabled
    ProcessCliQueue();

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
            logdev("Skipping reconnect, acc name or password not set");
        }
        else
        {   // everything fine, we have all data
            logdetail("Waiting %u ms before reconnecting.",GetConf()->reconnect);
            for(uint32 t = 0; t < GetConf()->reconnect && !this->Stopped(); t+=100) usleep(100);
            usleep(1000); // wait 1 sec before reconnecting
            CreateRealmSession();
        }
    }

    //logdetail("Disconnected, switching GUI back to Loginscreen.");

    // update currently existing/active sessions
    if(_rsession)
        _rsession->Update();
    if(_wsession)
        try { _wsession->Update(); } catch (...)
        {
            logerror("Unhandled exception in WorldSession::Update()");
        }


    // GetScripts()->GetEventMgr()->Update();

    usleep(GetConf()->networksleeptime);
}

void PseuInstance::ProcessCliQueue(void)
{
    std::string cmd;
    while(_cliQueue.size())
    {
        cmd = _cliQueue.front();
        _cliQueue.pop();
        try
        {
            //GetScripts()->RunSingleLine(cmd);
        }
        catch(...)
        {
            logerror("Exception while executing (HAB ICH AUS GEMACHT) CLI command: \"%s\"",cmd.c_str());
        }
    }
}

void PseuInstance::AddCliCommand(std::string cmd)
{
    _cliQueue.push(cmd);
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
    enablecli=false;
    enablegui=false;
    exitonerror=false;
    debug=0;
    rmcontrolport=0;
}

void PseuInstanceConf::ApplyFromVarSet(void)
{
    debug=3;
    realmlist="localhost";
    accname="admin";
    accpass="admin";
    exitonerror=false;
    reconnect=10000;
    realmport=3724;
    client=CLIENT_WOTLK;
    clientlang="enUS";
    realmname="Trinity";
    charname="Pseuwow";
    networksleeptime=1;
    showopcodes=3;
    hidefreqopcodes=true;
    hideDisabledOpcodes=true;
    enablecli=true;
    allowgamecmd=false;
    notifyping=true;
    showmyopcodes=true;
    disablespellcheck=false;
    enablegui=false;
    rmcontrolport=0;
    rmcontrolhost="";
    rmcontrolpass="";
    useMaps=false;
    skipaddonchat=true;
    dumpPackets=1;
    softquit=true;
    dataLoaderThreads=2;
    useMPQ=false;

    switch(client)
    {
      case CLIENT_CLASSIC_WOW:
      {
        clientbuild = 6005;
        clientversion_string="1.12.2";
        break;
      }
      case CLIENT_TBC:
      {
        clientbuild = 8606;
        clientversion_string="2.4.3";
        break;
      }
      case CLIENT_WOTLK:
      {
        clientbuild = 12340;
        clientversion_string="3.3.5";
        break;
      }
      case CLIENT_CATA:
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
    log_setloglevel(debug);
    log_setlogtime(true);
}




PseuInstanceConf::~PseuInstanceConf()
{
        //...
}




