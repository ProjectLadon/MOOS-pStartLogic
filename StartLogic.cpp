/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: StartLogic.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <chrono>
#include <string>
#include <vector>
#include "MBUtils.h"
#include "StartLogic.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

Condition::Condition(Value &conf, StartLogic* s) {
    node = s;
    if (conf.IsObject() &&
        conf.HasMember("msg") &&
        conf["msg"].IsString() &&
        conf.HasMember("truth_conditions") &&
        conf["truth_conditions"].IsArray()) {
            messageName = conf["msg"].GetString();
            for (auto &v: conf["truth_conditions"].GetArray()) {
                if (v.IsInt()) intTruthConditions.emplace_back(v.GetInt());
                if (v.IsString()) stringTruthConditions.emplace_back(v.GetString());
            }
    }
    if (conf.HasMember("delay") && conf["delay"].IsInt()) {
        delay = 1ms * conf["delay"].GetInt();
    }
    lastFalse = chrono::system_clock::now();
    firstTrue = lastFalse;
}

void Condition::registerVars() {node->registerVar(messageName);};

bool Condition::onNewMail(CMOOSMsg &msg) {
    if (msg.GetKey() == messageName) {

        // Check if the incoming message is a string or double and check if it's true
        bool tmpStatus = false;
        if (msg.IsDouble()) {
            int tmp = (int)msg.GetDouble();
            for (auto &c: intTruthConditions) {
                if (tmp == c) tmpStatus = true;
            }
        } else if (msg.IsString()) {
            string tmp = msg.GetString();
            for (auto &c: stringTruthConditions) {
                if (tmp == c) tmpStatus = true;
            }
        } else return false;

        // If we got a true signal, check that we've gone past the time delay
        if (tmpStatus) {
            if (lastFalse > firstTrue) firstTrue = chrono::system_clock::now();
            if ((chrono::system_clock::now() - firstTrue) > delay) {
                status = true;
            } else status = false;
        } else {
            lastFalse = chrono::system_clock::now();
            status = false;
        }
        return true;
    } else return false;
}

FixedMessage::FixedMessage(Value &conf, StartLogic* s) {
    node = s;
    if (conf.IsObject() &&
        conf.HasMember("msg") &&
        conf["msg"].IsString() &&
        conf.HasMember("on_output") &&
        conf.HasMember("off_output")) {
            messageName = conf["msg"].GetString();
            if (conf["on_output"].IsDouble()) {
                onOutputDouble = unique_ptr<double>(new double(conf["on_output"].GetDouble()));
            } else if (conf["on_output"].IsString()) {
                onOutputString = unique_ptr<string>(new string(conf["on_output"].GetString()));
            }
            if (conf["off_output"].IsDouble()) {
                offOutputDouble = unique_ptr<double>(new double(conf["off_output"].GetDouble()));
            } else if (conf["off_output"].IsString()) {
                offOutputString = unique_ptr<string>(new string(conf["off_output"].GetString()));
            }
    }
}

bool FixedMessage::transmit(bool status) {
    if (status) {
        if (onOutputString) {
            node->notify(messageName, *onOutputString);
        } else if (onOutputDouble) {
            node->notify(messageName, *onOutputDouble);
        }
    } else {
        if (offOutputString) {
            node->notify(messageName, *offOutputString);
        } else if (offOutputDouble) {
            node->notify(messageName, *offOutputDouble);
        }
    }
}

RetransmitMessage::RetransmitMessage(Value &conf, StartLogic* s) {
    node = s;
    if (conf.IsObject() &&
        conf.HasMember("input_msg") &&
        conf["input_msg"].IsString() &&
        conf.HasMember("output_msg") &&
        conf["output_msg"].IsString() &&
        conf.HasMember("default_msg")) {
            incoming = conf["input_msg"].GetString();
            outgoing = conf["output_msg"].GetString();
            if (conf["default_msg"].IsDouble()) {
                defaultDouble = unique_ptr<double>(new double(conf["default_msg"].GetDouble()));
            } else if (conf["default_msg"].IsString()) {
                defaultString = unique_ptr<string>(new string(conf["default_msg"].GetString()));
            }
    }
}

bool RetransmitMessage::onNewMail(CMOOSMsg &msg) {
    if (msg.GetKey() == incoming) {
        if (node->getState() == RunningState::RUNNING) {
            if (msg.IsDouble()) {
                node->notify(outgoing, msg.GetDouble());
            } else if (msg.IsString()) {
                node->notify(outgoing, msg.GetString());
            }
        } else {
            if (defaultDouble) {
                node->notify(outgoing, *defaultDouble);
            } else if (defaultString) {
                node->notify(outgoing, *defaultString);
            }
        }
    } else return false;
}

void RetransmitMessage::registerVars() {node->registerVar(incoming);};

//---------------------------------------------------------
// Procedure: OnNewMail

bool StartLogic::OnNewMail(MOOSMSG_LIST &NewMail) {
    MOOSMSG_LIST::iterator p;

    for(p=NewMail.begin(); p!=NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        for (auto &c: startConditions) {
            c->onNewMail(msg);
        }
        for (auto &c: stopConditions) {
            c->onNewMail(msg);
        }
        for (auto &r: retransmitMessages) {
            r->onNewMail(msg);
        }
    }

    return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool StartLogic::OnConnectToServer() {
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool StartLogic::Iterate() {
    for (auto &m: warningMessages) {
        if (state == RunningState::WARNING) {
            m->transmit(true);
        } else m->transmit(false);
    }
    for (auto &m: runningMessages) {
        if (state == RunningState::RUNNING) {
            m->transmit(true);
        } else m->transmit(false);
    }
    return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool StartLogic::OnStartUp() {
    list<string> sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
        list<string>::iterator p;
        for(p=sParams.begin(); p!=sParams.end(); p++) {
            string original_line = *p;
            string param = stripBlankEnds(toupper(biteString(*p, '=')));
            string value = stripBlankEnds(*p);
            Document conf;
            conf.Parse(value.c_str());

            if (param == "START_CONDITION") {
                startConditions.emplace_back(new Condition(conf, this));
            } else if (param == "STOP_CONDITION") {
                stopConditions.emplace_back(new Condition(conf, this));
            } else if (param == "WARNING_MESSAGE") {
                warningMessages.emplace_back(new FixedMessage(conf, this));
            } else if (param == "RUNNING_MESSAGE") {
                runningMessages.emplace_back(new FixedMessage(conf, this));
            } else if (param == "RETRANSMIT_MESSAGE") {
                retransmitMessages.emplace_back(new RetransmitMessage(conf, this));
            }
        }
    }

    RegisterVariables();
    return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void StartLogic::RegisterVariables() {
    for (auto &c: startConditions) c->registerVars();
    for (auto &c: stopConditions) c->registerVars();
    for (auto &c: retransmitMessages) c->registerVars();
}
