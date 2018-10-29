/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: StartLogic.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef StartLogic_HEADER
#define StartLogic_HEADER

#include <chrono>
#include <string>
#include <vector>
#include "MOOS/libMOOS/MOOSLib.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

class StartLogic;

enum class RunningState {
    IDLE,
    WARNING,
    RUNNING
};

class Condition {
    public:
        Condition(Value &conf, StartLogic* s);
        bool onNewMail(CMOOSMsg &msg);
        bool getStatus() {return status;};
        void registerVars();
        ~Condition() {};
    private:
        StartLogic* node;
        bool status = false;
        string messageName;
        vector<string> stringTruthConditions;
        vector<int> intTruthConditions;
        chrono::milliseconds delay = 0ms;
        chrono::system_clock::time_point lastFalse;
        chrono::system_clock::time_point firstTrue;
};

class FixedMessage {
    public:
        FixedMessage(Value &conf, StartLogic* s);
        bool transmit(bool status);
        ~FixedMessage() {};
    private:
        StartLogic* node;
        string messageName;
        unique_ptr<string> onOutputString;
        unique_ptr<string> offOutputString;
        unique_ptr<double> onOutputDouble;
        unique_ptr<double> offOutputDouble;

};

class RetransmitMessage {
    public:
        RetransmitMessage(Value &conf, StartLogic* s);
        bool onNewMail(CMOOSMsg &msg);
        void registerVars();
        ~RetransmitMessage() {};
    private:
        StartLogic* node;
        string incoming;
        string outgoing;
        unique_ptr<string> defaultString;
        unique_ptr<double> defaultDouble;
};

class StartLogic : public CMOOSApp {
    public:
        StartLogic() {};
        ~StartLogic() {};
        RunningState getState() {return state;};
        bool notify(const string &sVar, const string &sVal) {Notify(sVar, sVal);};
        bool notify(const string &sVar, const double &sVal) {Notify(sVar, sVal);};
        bool registerVar(const string &sVar) {Register(sVar);};

    protected: // Standard MOOSApp functions to overload
        bool OnNewMail(MOOSMSG_LIST &NewMail);
        bool Iterate();
        bool OnConnectToServer();
        bool OnStartUp();

    protected:
        void RegisterVariables();

    private: // Configuration variables
        RunningState state = RunningState::IDLE;
        vector<unique_ptr<Condition>> startConditions;
        vector<unique_ptr<Condition>> stopConditions;
        vector<unique_ptr<FixedMessage>> warningMessages;
        vector<unique_ptr<FixedMessage>> runningMessages;
        vector<unique_ptr<RetransmitMessage>> retransmitMessages;

    private: // State variables
};

#endif
