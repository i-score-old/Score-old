#include "TTModular.h"
#include "TTScore.h"

#include <iostream>
#include <string>

// A class for our application
class DemoApp {
    
public:
    
    DemoApp(){};
    ~DemoApp(){};
    
    // This application is divided into four main functions
    void SetupModular();
    void SetupScore();
    void Export();
    void Execute(std::string command);
    void Quit();
    
private:
    
    // Declare the application manager, our application and another one
    TTObject mApplicationManager;
    TTObject mApplicationDemo;
    TTObject mApplicationRemote;
    
    // Declare protocol units to use
    TTObject mProtocolMinuit;
    TTObject mProtocolWebSocket;
 
public:
    
    // Declare publicly all datas of our application to retreive them from the callback function
    TTObject mDataDemoParameter;     // a parameter is relative to the state of our application
    TTObject mDataDemoMessage;       // a message is a kind of command to send to our application
    TTObject mDataDemoReturn;        // a return is a kind of notification sent by our application
    
    // Declare publicly the scenario to retreive it from the callback function
    TTObject mScenario;
    
    // Be friend with the callback function
    friend TTErr DemoAppDataReturnValueCallback(const TTValue& baton, const TTValue& v);
};

// Callback function to get data's value back
TTErr DemoAppDataReturnValueCallback(const TTValue& baton, const TTValue& v);

int
main(int argc, char **argv) 
{
    DemoApp app;

    TTLogMessage("\n*** Start of Jamoma Modular and Score demonstration ***\n");
    
    app.SetupModular();
    app.SetupScore();
    
    // read command from console
    do {
        TTLogMessage("\nType a command : \n");
        
        std::string s;
        std::getline(std::cin, s);
        
        // quit the application
        if (!s.compare("quit")) {
            
            app.Quit();
            
            TTLogMessage("\n*** End of Jamoma Modular and Score demonstration ***\n");
            return EXIT_SUCCESS;
        }
        // dump informations about the application
        else if (!s.compare("export")) {
            
             app.Export();
        }
        // parse a command and execute it
        else {
            
            app.Execute(s);
        }
    }
    while (YES);
}

void
DemoApp::SetupModular()
{
    TTValue     args, v, out, none;
    TTAddress   address;
    TTErr       err;
    
    TTLogMessage("\n*** Initialisation of Modular environnement ***\n");
    /////////////////////////////////////////////////////////////////////
    
    // Init the Modular library (passing the folder path where all the dylibs are)
    TTModularInit("/usr/local/jamoma");
    
    // Create an application manager
    mApplicationManager = TTObject("ApplicationManager");
    
    
    TTLogMessage("\n*** Creation of mApplicationDemo and mApplicationRemote applications ***\n");
    ////////////////////////////////////////////////////////////////////////////////
    
    // Create a local application called "demo" and get it back
    err = mApplicationManager.send("ApplicationInstantiateLocal", "demo", out);
    
    if (err) {
        TTLogError("Error : can't create demo application \n");
        return;
    }
    else
        mApplicationDemo = out[0];
    
    
    // Create a distant application called "max" and get it back
    err = mApplicationManager.send("ApplicationInstantiateDistant", "Jamoma", out);
    
    if (err) {
        TTLogError("Error : can't create Max application \n");
        return;
    }
    else
        mApplicationRemote = out[0];
    
    // Get registered application names
    mApplicationManager.get("applicationNames", out);
    for (TTElementIter it = out.begin() ; it != out.end() ; it++) {
        TTSymbol name = TTElement(*it);
        TTLogMessage("%s application is well registered into the application manager \n", name.c_str());
    }
    

    TTLogMessage("\n*** Enable Minuit communication ***\n");
    ////////////////////////////////////////////////////////////////////////
    
    // Create a Minuit protocol unit
    err = mApplicationManager.send("ProtocolInstantiate", "Minuit", out);
    
    if (err) {
        TTLogError("Error : can't create Minuit protocol unit \n");
        return;
    }
    else
        mProtocolMinuit = out[0];
    
    // Get Minuit Protocol attribute names and types
    mProtocolMinuit.get("parameterNames", out);
    for (TTElementIter it = out.begin() ; it != out.end() ; it++) {
        TTSymbol name = TTElement(*it);
        TTSymbol type = mProtocolMinuit.attributeType(name);
        TTLogMessage("Minuit %s parameter is a %s \n", name.c_str(), type.c_str());
    }
    
    // Register mApplicationDemo and mApplicationRemote to the Minuit protocol
    mProtocolMinuit.send("ApplicationRegister", "demo", out);
    mProtocolMinuit.send("ApplicationRegister", "Jamoma", out);
    
    // Select mApplicationDemo to set its protocol parameters
    mProtocolMinuit.send("ApplicationSelect", "demo", out);
    mProtocolMinuit.set("port", 13579);
    mProtocolMinuit.set("ip", "127.0.0.1");
    
    // Select mApplicationRemote to set its protocol parameters
    mProtocolMinuit.send("ApplicationSelect", "Jamoma", out);
    mProtocolMinuit.set("port", 9998);
    mProtocolMinuit.set("ip", "127.0.0.1");
    
    // Get Minuit parameters for each registered application
    mProtocolMinuit.get("applicationNames", out);
    for (TTElementIter it = out.begin() ; it != out.end() ; it++) {
        TTSymbol name = TTElement(*it);
        
        mProtocolMinuit.send("ApplicationSelect", name, out);
        TTLogMessage("Minuit setup for %s application : \n", name.c_str());
        
        mProtocolMinuit.get("ip", v);
        TTSymbol ip = v[0];
        TTLogMessage("- ip = %s \n", ip.c_str());
        
        mProtocolMinuit.get("port", v);
        TTUInt16 port = v[0];
        TTLogMessage("- port = %d \n", port);
    }
    
    // Enable Minuit communication
    mProtocolMinuit.send("Run");
    
    
	TTLogMessage("\n*** Creation of mApplicationDemo datas ***\n");
    /////////////////////////////////////////////////////////
    
    // Create a parameter data and set its callback function and baton and some attributes
    mDataDemoParameter = TTObject("Data", "parameter");
    
    // Setup the callback mechanism to get the value back
    args = TTValue(TTPtr(this), mDataDemoParameter);
    mDataDemoParameter.set("baton", args);
    mDataDemoParameter.set("function", TTPtr(&DemoAppDataReturnValueCallback));

    // Setup the data attributes depending of its use inside the application
    mDataDemoParameter.set("type", "decimal");
    mDataDemoParameter.set("rangeBounds", TTValue(0., 1.));
    mDataDemoParameter.set("rangeClipmode", "low");
    mDataDemoParameter.set("rampDrive", "System");
    mDataDemoParameter.set("description", "control the speed of the scenario");
    
    // Register the parameter data into mApplicationDemo at an address
    args = TTValue("/rate", mDataDemoParameter);
    mApplicationDemo.send("ObjectRegister", args, out);
    
    
    // Create a message data and set its callback function and baton and some attributes
    mDataDemoMessage = TTObject("Data", "message");
    
    // Setup the callback mechanism to get the value back
    args = TTValue(TTPtr(this), mDataDemoMessage);
    mDataDemoMessage.set("baton", args);
    mDataDemoMessage.set("function", TTPtr(&DemoAppDataReturnValueCallback));
    
    // Setup the data attributes depending of its use inside the application
    mDataDemoMessage.set("type", "none");
    mDataDemoMessage.set("description", "start the playing of the scenario from the beginning");
    
    // Register the message data into mApplicationDemo at an address
    args = TTValue("/play", mDataDemoMessage);
    mApplicationDemo.send("ObjectRegister", args, out);
    
    
    // Create a return data and set its callback function and baton and some attributes
    mDataDemoReturn = TTObject("Data", "return");
    
    // Setup the callback mechanism to get the value back
    args = TTValue(TTPtr(this), mDataDemoReturn);
    mDataDemoReturn.set("baton", args);
    mDataDemoReturn.set("function", TTPtr(&DemoAppDataReturnValueCallback));

    // Setup the data attributes depending of its use inside the application
    mDataDemoReturn.set("type", "integer");
    mDataDemoReturn.set("description", "return the time progression in millis second");
    
    // Register the return data into mApplicationDemo at an address
    args = TTValue("/time", mDataDemoReturn);
    mApplicationDemo.send("ObjectRegister", args, out);

    
	TTLogMessage("\n*** Exploration of mApplicationRemote datas ***\n");
    /////////////////////////////////////////////////////////
    
    // Explore the mApplicationRemote
    mApplicationRemote.send("DirectoryBuild");
}

void
DemoApp::SetupScore()
{
    TTLogMessage("\n*** Initialisation of Score environnement ***\n");
    /////////////////////////////////////////////////////////////////////
    
    // Init the Score library (passing the folder path where all the dylibs are)
    TTScoreInit("/usr/local/jamoma");
    
    
    TTLogMessage("\n*** Creation of an interactive scenario ***\n");
    ////////////////////////////////////////////////////////////////////////////////
    
    // Create two events for the start and an end at 1 minute
    TTObject start("TimeEvent");
    start.set("name", TTSymbol("start of the demonstration"));
    
    TTObject end("TimeEvent", 60000u);
    end.set("name", TTSymbol("end of the demonstration"));
    
    // Create a Scenario passing the start and end events
    TTValue args(start, end);
    mScenario = TTObject("Scenario", args);
    
    
    TTLogMessage("\n*** Creation of events inside a scenario ***\n");
    ////////////////////////////////////////////////////////////////////////////////
    TTObject    event1, event2;
    TTValue     out;
    
    // edit an event at 2 seconds
    mScenario.send("TimeEventCreate", 2000u, out);
    event1 = out[0];
    event1.set("name", TTSymbol("start of a fade in"));
    
    // edit an event at 6 seconds
    mScenario.send("TimeEventCreate", 6000u, out);
    event2 = out[0];
    event2.set("name", TTSymbol("end of the fade in"));
    
    
    TTLogMessage("\n*** Edit state of each event ***\n");
    ////////////////////////////////////////////////////////////////////////////////
    TTObject    state;
 
    
}

void
DemoApp::Export()
{
    TTValue     args, out;
    TTObject    xmlHandler("XmlHandler");
    
    // export Modular setup and the Score scenario
    args = TTValue(mApplicationManager, mScenario);
    xmlHandler.set("object", mApplicationDemo);
    xmlHandler.send("Write", "./mApplicationDemo.xml", out);
}

void
DemoApp::Execute(std::string command)
{
    // parse the command : address value
    TTValue v = TTString(command);
    v.fromString();
    
    // a command have to start by a symbol
    if (v.size() > 1) {
        
        if (v[0].type() == kTypeSymbol) {
            
            TTSymbol    address = v[0];
            TTValue     args, out, none;
            
            args.copyFrom(v, 1);
            
            // Retreive a data object
            TTErr err = mApplicationDemo.send("ObjectRetreive", address, out);
            
            if (!err) {
                
                TTObject aData = out[0];
                
                // Set data's value and check his validity
                aData.send("Command", args, none);
            }
        }
    }
}

void
DemoApp::Quit()
{
    TTValue out;

    TTLogMessage("\n*** Release mApplicationDemo datas ***\n");
    /////////////////////////////////////////////////////
    
    // Unregister the parameter
    if ( mApplicationDemo.send("ObjectUnregister", "/myParameter", out))
        TTLogError("Error : can't unregister data at /myParameter address \n");
    
    // Unregister the message
    if (mApplicationDemo.send("ObjectUnregister", "/myMessage", out))
        TTLogError("Error : can't unregister data at /myMessage address \n");
    
    // Unregister the return
    if (mApplicationDemo.send("ObjectUnregister", "/myReturn", out))
        TTLogError("Error : can't unregister data at /myReturn address \n");
    
    
    TTLogMessage("\n*** Release protocols ***\n");
    ///////////////////////////////////////////////
    
    mApplicationManager.send("ProtocolRelease", "Minuit", out);
    //mApplicationManager.send("ProtocolRelease", "WebSocket", out);
    
    
    TTLogMessage("\n*** Release applications ***\n");
    //////////////////////////////////////////////////
    
    mApplicationManager.send("ApplicationRelease", "i-score", out);
    mApplicationManager.send("ApplicationRelease", "demo", out);
}

TTErr
DemoAppDataReturnValueCallback(const TTValue& baton, const TTValue& value)
{
    DemoApp*    demoApp = (DemoApp*)TTPtr(baton[0]);
    TTObject    anObject = baton[1];
    
	// Reteive which data has been updated
    if (anObject.instance() == demoApp->mDataDemoParameter.instance()) {
        
        // print the returned value
        TTLogMessage("/speed has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    if (anObject.instance() == demoApp->mDataDemoMessage.instance()) {
        
        // print the returned value
        TTLogMessage("/play has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    if (anObject.instance() == demoApp->mDataDemoReturn.instance()) {
        
        // print the returned value
        TTLogMessage("/time has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}