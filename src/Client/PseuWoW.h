
#ifndef _PSEUWOW_H
#define _PSEUWOW_H

#include "common.h"
#include "HelperDefs.h"
#include "Auth/BigNumber.h"
#include "Network/SocketHandler.h"
#include <queue>

class RealmSession;
class WorldSession;
class Sockethandler;
class PseuInstanceRunnable;

// possible conditions threads can wait for. used for thread synchronisation. extend if needed.
enum InstanceConditions
{
    COND_GUI_INITIALIZED,
    COND_GUI_SCENE_CHANGED,
    COND_GUI_CLOSED,
    COND_MAP_LOADED,

    COND_MAX
};

enum Client
{
    CLIENT_UNKNOWN,
    CLIENT_CLASSIC_WOW,
    CLIENT_TBC,
    CLIENT_WOTLK,
    CLIENT_CATA
};

class PseuInstanceConf
{
    public:

    PseuInstanceConf();
    ~PseuInstanceConf();
    void ApplyFromVarSet();


    uint8 debug;
    std::string realmlist;
    std::string accname;
    std::string accpass;
    bool exitonerror;
    uint32 reconnect;
    uint16 realmport;
    uint16 worldport;
    uint8 client;
    uint8 clientversion[3];
    std::string clientversion_string;
    uint16 clientbuild;
    std::string clientlang;
    std::string realmname;
    std::string charname;
    std::string worldhost;
    uint16 networksleeptime;
    uint8 showopcodes;
    bool hidefreqopcodes;
    bool hideDisabledOpcodes;
    bool allowgamecmd;
    bool enablecli;
    bool notifyping;
    bool showmyopcodes;
    bool disablespellcheck;
    uint32 rmcontrolport;
    std::string rmcontrolhost;
    std::string rmcontrolpass;
    bool useMaps;
    bool skipaddonchat;
    uint8 dumpPackets;
    bool softquit;
    uint8 dataLoaderThreads;
};


class PseuInstance
{
public:

    PseuInstance(PseuInstanceRunnable *run);
    ~PseuInstance();


    inline WorldSession *GetWSession(void) { return _wsession; }
    inline RealmSession *GetRSession(void) { return _rsession; }
    inline PseuInstanceConf *GetConf(void) { return _conf; }

    bool ConnectToRealm(void);

    inline void SetScpDir(std::string dir) { _scpdir = dir; }
    inline void SetSessionKey(BigNumber key) { _sessionkey = key; }
    inline BigNumber *GetSessionKey(void) { return &_sessionkey; }
    inline void SetError(void) { _error = true; }

    bool Init(void);
    inline void Stop(void) { _stop = true; }
    inline bool Stopped(void) { return _stop; }
    inline void SetFastQuit(bool q=true) { _fastquit=true; }
    int Run(void);
    void Update(void);
    void Sleep(uint32 msecs);

    inline void CreateWorldSession(void) { _createws = true; }
    inline void CreateRealmSession(void) { _creaters = true; }

    void ProcessCliQueue(void);
    void AddCliCommand(std::string);

    void WaitForCondition(InstanceConditions c, uint32 timeout = 0);
    void SaveAllCache(void);

private:

    PseuInstanceRunnable *_runnable;
    RealmSession *_rsession;
    WorldSession *_wsession;
    PseuInstanceConf *_conf;
    std::string _confdir,_scpdir; // _scpdir is the scripts dir, and NOT where SCP files are stored!!
    bool _initialized;
    bool _stop,_fastquit;
    bool _startrealm;
    bool _error;
    bool _createws, _creaters; // must create world/realm session?
    BigNumber _sessionkey;
    const char *_ver,*_ver_short;
    SocketHandler _sh;
    std::queue<std::string> _cliQueue;

};


#endif
