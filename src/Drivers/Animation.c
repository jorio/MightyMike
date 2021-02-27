/****************************/
/*   ANIMATION              */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/

/****************************/
/*    EXTERNALS             */
/****************************/

#include "myglobals.h"
#include "object.h"
#include "misc.h"
#include "sound2.h"


extern	ObjNode	*gThisNodePtr;
extern	Ptr		gSHAPE_HEADER_Ptrs[MAX_SHAPES_IN_FILE];
extern	Boolean	gGlobalFlagList[MAX_GLOBAL_FLAGS];
extern	short	gEnemyFreezeTimer;

/****************************/
/*    CONSTANTS             */
/****************************/

enum
{
	ANIMOP_NOP,
	ANIMOP_FRAME,
	ANIMOP_END,
	ANIMOP_LOOP,
	ANIMOP_SPEED,
	ANIMOP_GOTO,
	ANIMOP_GOTOANIM,
	ANIMOP_SETFLAG,
	ANIMOP_PAUSE,
	ANIMOP_DELETE,
	ANIMOP_GLOBALSETFLAG,
	ANIMOP_PLAYSOUND
};



/**********************/
/*     VARIABLES      */
/**********************/


/************************ ANIMATE A SPRITE ********************/
//
// NOTE: node->AnimsList actually points to the ANIM_LIST+2, it skips the
//	 "# anims" word.
//

void AnimateASprite(ObjNode *theNodePtr)
{
Ptr			animDataPtr;
bool		doMore;
Ptr			tempPtr;
int32_t		offset;

	if	(theNodePtr->AnimsList == nil)					// exit if no anim list
		return;

	if (gEnemyFreezeTimer)								// dont animate frozen enemies
		if (theNodePtr->CType & CTYPE_ENEMYA)
			return;

	theNodePtr->AnimCount -= (theNodePtr->AnimSpeed);	// dec the counter too see if do anim
	if (theNodePtr->AnimCount > 0)
		return;

	theNodePtr->AnimCount = theNodePtr->AnimConst;		// reset counter


	do
	{
		doMore = false;

		offset = *(int32_t*) (theNodePtr->AnimsList + (theNodePtr->SubType<<2));	// get offset to ANIM_DATA
		animDataPtr	=	(theNodePtr->SHAPE_HEADER_Ptr+offset+1);		// get ptr to ANIM_DATA
		animDataPtr +=  (theNodePtr->AnimLine++) << 2;

		int16_t opcode	= *(int16_t*) (animDataPtr+0);
		int16_t operand	= *(int16_t*) (animDataPtr+2);

		switch (opcode)
		{
			case	ANIMOP_FRAME:
					theNodePtr->CurrentFrame = operand;
					break;

			case	ANIMOP_LOOP:
					theNodePtr->AnimLine = 0;				// reset data offset
					doMore = true;
					break;

			case	ANIMOP_SPEED:
					theNodePtr->AnimConst =
					theNodePtr->AnimCount = operand;
					doMore = true;
					break;

			case	ANIMOP_END:
					theNodePtr->AnimLine--;				// dont go to next opcode
					theNodePtr->AnimConst = 0xffff;		// slowest speed
					break;

			case	ANIMOP_SETFLAG:
					tempPtr = (Ptr)&(theNodePtr->Flag0);
					tempPtr += operand;
					*tempPtr = true;
					doMore = true;
					break;

			case	ANIMOP_PAUSE:
					theNodePtr->AnimCount = operand<<8;
					theNodePtr->AnimSpeed = 0x100;			// count 1 tick
					break;

			case	ANIMOP_GOTO:
					theNodePtr->AnimLine = operand;			// set data offset
					doMore = true;
					break;

			case	ANIMOP_GOTOANIM:
					SwitchAnim(theNodePtr,operand);			// switch anim
					doMore = true;
					break;

			case	ANIMOP_DELETE:
					DeleteObject(theNodePtr);
					break;

			case	ANIMOP_GLOBALSETFLAG:
					tempPtr = (Ptr)&(gGlobalFlagList[0]);
					tempPtr += operand;
					*tempPtr = true;
					doMore = true;
					break;

			case	ANIMOP_PLAYSOUND:
					PlaySound(operand);
					doMore = true;
					break;

			}
	}
	while (doMore);
}

/********************* SWITCH ANIM *********************/

void SwitchAnim(ObjNode	*theNodePtr,short animNum)
{
	theNodePtr->SubType = animNum;
	theNodePtr->AnimCount = theNodePtr->AnimLine = 0;		// reset animation stuff
	theNodePtr->AnimConst = theNodePtr->AnimSpeed = 0x100;
}





