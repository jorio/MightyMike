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


extern ObjNode	*MakeShadow(ObjNode *, Byte);
extern void	MoveShadow(void);
extern void	MakeMikeMessage(short);
extern void	MoveMessage(void);
extern void	PutPlayerSignal(short);
extern void	MovePlayerSignal(void);
extern void	MovePlayerSignalOHM(void);
extern void	MakeSplash(short, short, short);
extern Boolean	AddKeyColor(ObjectEntryType *);
