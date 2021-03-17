//
// MiscAnims.h
//

enum
{
	MESSAGE_NUM_IVECOMETOSAVEYOU,
	MESSAGE_NUM_COMEHERERODENT,
	MESSAGE_NUM_TAKETHAT,
	MESSAGE_NUM_EATMYDUST,
	MESSAGE_NUM_FOOD,
	MESSAGE_NUM_FREEDUDE,
	MESSAGE_NUM_OUCH,
	MESSAGE_NUM_NICEGUY,
	MESSAGE_NUM_FIREHOLE
};


enum
{
	SHADOWSIZE_TINY,
	SHADOWSIZE_SMALL,
	SHADOWSIZE_MEDIUM,
	SHADOWSIZE_LARGE,
	SHADOWSIZE_GIANT
};


ObjNode	*MakeShadow(ObjNode *, Byte);
void	MoveShadow(void);
void	MakeMikeMessage(short);
void	MoveMessage(void);
void	PutPlayerSignal(short);
void	MovePlayerSignal(void);
void	MovePlayerSignalOHM(void);
void	MakeSplash(short, short, short);
Boolean	AddKeyColor(ObjectEntryType *);
