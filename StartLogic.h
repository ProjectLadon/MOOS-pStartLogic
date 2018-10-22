/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: StartLogic.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef StartLogic_HEADER
#define StartLogic_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class StartLogic : public CMOOSApp
{
 public:
   StartLogic();
   ~StartLogic();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();

 private: // Configuration variables

 private: // State variables
};

#endif 
