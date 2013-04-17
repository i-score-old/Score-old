/*
* Interface to ask the CSP
* Copyright © 2013, Théo de la Hogue, Clément Bossut
*
* License: This code is licensed under the terms of the "New BSD License"
* http://creativecommons.org/licenses/BSD/
*/


#ifndef Scenario_CSPAsker_h
#define Scenario_CSPAsker_h

class CSPAsker
{
public:
    virtual int getMaxValue() =0;
    
    virtual void moveReport(void *pProcess, int newStart, int newEnd) =0;
};

#endif
