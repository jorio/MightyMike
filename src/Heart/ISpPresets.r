//
//
// ISpPresets.r
//
//

#include "InputSprocket.r"

/* the InputSprocket application resource tells utility programs how the */
/* application uses InputSprocket */

resource 'isap' (128)
{
	callsISpInit,
	usesInputSprocket
};


/* the set list resource contains the list of all the saved sets for devices */
/* that are provided in the application's resource fork */


resource 'setl' (1000)
{
	currentVersion,
	{
		"Default (Keyboard)", 0, kISpDeviceClass_Keyboard,
		kISpKeyboardID_Apple, notApplSet, isDefaultSet,
		128,
	};
};


/* Default keyboard set */

resource 'tset' (128, "Default (Keyboard)")
{
	supportedVersion,
	{	
		spaceKey,				// Fire/attack
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		commandKey,			    // select weapon
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		upKey,					// DPad Forward
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		downKey,				// DPad Down
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		leftKey,				// DPad Left
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		rightKey,				// DPad Right
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		rKey,					// Radar
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		escKey,					// Pause
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		 		 
		mKey,					// Toggle Music
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		eKey,					// Toggle Effects
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		 
		plusKey,				// Raise Volume
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,

		minusKey,				// Lower Volume
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOff,
		 
		0x0c,					// Quit App
		 rControlOff, rOptionOff, rShiftOff, controlOff, optionOff, shiftOff, commandOn
	};
};




