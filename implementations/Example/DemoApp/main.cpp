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
    void Start();
    void Parse(std::string command);
    void Quit();
    
private:
    
    // Declare the application manager and our application
    TTObject mApplicationManager;
    TTObject mApplicationDemo;
    
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
        // run the interactive scenario
        else if (!s.compare("start")) {
            
            app.Start();
        }
        // parse a command and execute it
        else {
            
            app.Parse(s);
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
    args = TTValue("/myParameter", mDataDemoParameter);
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
    args = TTValue("/myMessage", mDataDemoMessage);
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
    args = TTValue("/myReturn", mDataDemoReturn);
    mApplicationDemo.send("ObjectRegister", args, out);
}

void
DemoApp::SetupScore()
{
    TTValue     out;
    TTObject    xmlHandler("XmlHandler");
    
    TTLogMessage("\n*** Initialisation of Score environnement ***\n");
    /////////////////////////////////////////////////////////////////////
    
    // Init the Score library (passing the folder path where all the dylibs are)
    TTScoreInit("/usr/local/jamoma");
    
    
    TTLogMessage("\n*** Reading of an interactive scenario file ***\n");
    ////////////////////////////////////////////////////////////////////////////////
    
    // Create an empty Scenario
    mScenario = TTObject("Scenario");
    
    // Read DemoScenario.score file to fill mScenario
    xmlHandler.set("object", mScenario);
    xmlHandler.send("Read", "../DemoScenario.score", out);
}

void
DemoApp::Start()
{
    mScenario.send("Start");
}

void
DemoApp::Parse(std::string command)
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
    
    
    TTLogMessage("\n*** Release application ***\n");
    //////////////////////////////////////////////////
    
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
        TTLogMessage("/myParameter has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    if (anObject.instance() == demoApp->mDataDemoMessage.instance()) {
        
        // print the returned value
        TTLogMessage("/myMessage has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    if (anObject.instance() == demoApp->mDataDemoReturn.instance()) {
        
        // print the returned value
        TTLogMessage("/myReturn has been updated to %s \n", value.toString().data());
        return kTTErrNone;
    }
    
    return kTTErrGeneric;
}