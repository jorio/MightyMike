//
// Triggers.h
//

enum
{
	TRIGTYPE_TELEPORT,
	TRIGTYPE_DOOR,
	TRIGTYPE_FAIRYDOOR,
	TRIGTYPE_BARGAINDOOR
};


#define	TriggerSides	Kind
#define	TriggerType		Flag3

extern Boolean	HandleTrigger(ObjNode *, Byte);
extern Boolean	AddTeleport(ObjectEntryType *);
extern Boolean	DoTrig_Teleport(void);
extern Boolean	AddClowndoor(ObjectEntryType *);
extern Boolean	AddCandyDoor(ObjectEntryType *);
extern Boolean	AddJurassicDoor(ObjectEntryType *);
extern Boolean	AddBargainDoor(ObjectEntryType *);
extern void	MoveBargainDoor(void);
extern Boolean	AddFairyDoor(ObjectEntryType *);
extern void	MoveFairyDoor(void);
extern Boolean	DoTrig_FairyDoor(void);
extern Boolean	DoTrig_Door(void);
extern Boolean	DoTrig_BargainDoor(void);
