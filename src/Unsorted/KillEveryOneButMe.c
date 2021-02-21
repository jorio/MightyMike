/* KillEveryBodyButMe.c */
/* Some folks want to kill all the other applications running on a machine */
/* for demos, school situations, kiosks, etc. */
/* with System 7, it's easy, just use the Process Manager and AppleEvents. */
/* This thing shows you how. */
/* PLEASE don't abuse this.  System 7 gives the user full-time multiFinder, and they */
/* like it, our System Software team worked very hard to make this */
/* happen.  ONLY do this in very special circumstances, or you're taking */
/* power away from the user and weaking the strength of the Mac. */
/* Written  by C.K. Haun <TR> */
/* Apple Developer Tech Support */

/* BUG FIX, if only this app and the Finder were/was running, it didn't do anything */
/* Ooops */

/* Of course, Copyright 1991-1992, Apple Computer Inc. */

#include "myglobals.h"
#include "misc.h"


void KillEveryBody(void);
static void AskIfOkay(void);


/*********************** KILL EVERY BODY ******************************/
/* This is the killer code.  It finds and kills every other  */
/* application on your machine. */

void KillEveryBody(void)
{
ProcessSerialNumber myProc, processSN;
ProcessSerialNumber finderPSN;
ProcessInfoRec infoRec;
Str31 processName;
FSSpec procSpec;

OSErr myErr = noErr;
OSErr otherError;
AppleEvent theEvent;
AEDesc theAddress;
Boolean ourFlag, notFinder;
Boolean finderFound = false;
Boolean gHasAppleEvents = false;
long	aLong;

	AskIfOkay();

    /* Check this machine for AppleEvents.  If they are not here (ie not 7.0) then we exit */

    gHasAppleEvents = (Gestalt(gestaltAppleEventsAttr, &aLong) == noErr);
	if (!gHasAppleEvents)
	{
		return;
	}

    GetCurrentProcess(&myProc);

    /* Preset the PSN to no PSN, see IM VI, the Process Manager */

    processSN.lowLongOfPSN = kNoProcess;
    processSN.highLongOfPSN = kNoProcess;
    finderPSN.lowLongOfPSN = (unsigned long)nil;
    finderPSN.highLongOfPSN = (unsigned long)nil;

    do
    {
        myErr = GetNextProcess(&processSN);
        if (myErr == procNotFound)											// see if done
        	break;
        if (myErr)															// see if other error
        	DoFatalAlert("GetNextProcess failed!");

			/* See if it's us first */

		notFinder = true;
        myErr = SameProcess(&myProc, &processSN, &ourFlag);
        if (myErr)
        	DoFatalAlert("SameProcess failed!");

        infoRec.processInfoLength = sizeof(ProcessInfoRec);
        infoRec.processName = &processName[0];
        infoRec.processAppSpec = &procSpec;
        myErr = GetProcessInformation(&processSN, &infoRec);
        if (myErr)
        	DoFatalAlert("GetProcessInformation failed!");


        if (!ourFlag && !finderFound)
        {
            /* see if it's the Finder, we have to kill the finder LAST */
            /* or else non-sys 7 apps won't get killed */
            /* since the Finder must be there to convert the AppleEvent to Puppet Strings */
            /* if the app is not APpleEvent aware */
			/* Also, FileShare HAS to be killed before the Finder */
			/* or your life will be unpleasant */

            if (infoRec.processSignature == 'MACS' && infoRec.processType == 'FNDR')
            {
        	        /* save this number for later  */
                finderPSN = processSN;
                notFinder = false;
                finderFound = true;

            }
            else
            {
                notFinder = true;
            }
        }

        if (!myErr && !ourFlag && notFinder)
        {
            otherError = AECreateDesc(typeProcessSerialNumber, (Ptr)&processSN, sizeof(processSN), &theAddress);
            if (otherError)
         		DoFatalAlert("AECreateDesc failed!");

      		otherError = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &theAddress, kAutoGenerateReturnID,
                                                kAnyTransactionID, &theEvent);
	        if (otherError)
    	    	DoFatalAlert("AECreateDesc failed!");

            AEDisposeDesc(&theAddress);

            /* Again, the Finder will convert the AppleEvent to puppetstrings if */
            /* the application is a System 6 or non-AE aware app.  This ONLY  */
            /* happens for the 4 required (oapp,odoc,pdoc, and quit) AppleEvents  */
            /* and ONLY if you use the PSN for the address */

            AESend(&theEvent, nil, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer,
              		 kAENormalPriority, kAEDefaultTimeout,  nil, nil);

            AEDisposeDesc(&theEvent);
        }
    } while (!myErr);

    /* Now, if the finder was running, it's safe to kill it */

    if (finderPSN.lowLongOfPSN || finderPSN.highLongOfPSN)
    {
        otherError = AECreateDesc(typeProcessSerialNumber, (Ptr)&finderPSN, sizeof(processSN), &theAddress);
        if (!otherError)
            otherError = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &theAddress, kAutoGenerateReturnID,
                                            kAnyTransactionID, &theEvent);
        if (!otherError)
            AEDisposeDesc(&theAddress);
        if (!otherError)
            AESend(&theEvent, nil, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer, kAENormalPriority, kAEDefaultTimeout, nil,
                   nil);
        AEDisposeDesc(&theEvent);
    }


		/* WAIT FOR EVENT TO LET OTHER APPS PROCESS OUR AE */
	{
		EventRecord	theEvent;
		short	i;

		for (i=0; i < 5; i++)
			WaitNextEvent(everyEvent, &theEvent, 50, nil);
	}
}



//============================================================


/********************* ASK IF OKAY **********************/

static void AskIfOkay(void)
{
DialogPtr	myDialog;
short		itemHit;
Boolean		dialogDone;

	myDialog = GetNewDialog(500,nil,MOVE_TO_FRONT);

	dialogDone = FALSE;
	while(dialogDone == FALSE)
	{
		ModalDialog(nil, &itemHit);
		switch (itemHit)
		{
			case 1:
					dialogDone = TRUE;
					break;

			case 2:
					CleanQuit();
					break;

		}
	}
	DisposDialog(myDialog);
}


